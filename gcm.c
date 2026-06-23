#include <stdint.h>
#include "aes.h"
#include "gcm.h"

// For XORing the GHASH accumulator X and the ciphertexts and AAD
// dest is the GHASH accumulator which starts with 0^128 (128 bits of 0s), and it gets XORed with AAD padded to 16 bytes first, and then with ciphertexts subsequently
static void xor_block(uint8_t *dest, const uint8_t *src) {
    for (int i = 0; i < 16; i++) {
        dest[i] ^= src[i];
    }
}

//For Galois Field multiplication when MSB is 1
static void shift_right_block(uint8_t *v) {
    uint8_t carry = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t next_carry = (v[i] & 1) ? 0x80 : 0;
        v[i] = (v[i] >> 1) | carry;
        carry = next_carry;
    }
}

//Galois field multiplication (GF(2^128))
static void gf_mult(const uint8_t *X, const uint8_t *Y, uint8_t *output) {
    uint8_t Z[16] = {0};
    uint8_t V[16];
    for (int i = 0; i < 16; i++) V[i] = Y[i];

    for (int i = 0; i < 16; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            if (X[i] & (1 << bit)) xor_block(Z, V);
            uint8_t lsb = V[15] & 1;
            shift_right_block(V);
            if (lsb) V[0] ^= 0xE1;
        }
    }
    for (int i = 0; i < 16; i++) output[i] = Z[i];
}
//Creation of counter block
static void inc32(uint8_t *counter_block) {
    for (int i = 15; i >= 12; i--) {
        counter_block[i]++;
        if (counter_block[i] != 0) break;
    }
}
//Counter mode for GCM (No authentication yet)
//iv is the initialisation vector/nonce which is 12 bytes : First 8 bytes are 0s and next 4 bytes are from seq
static void gcm_encrypt_ctr(
    const uint8_t *expanded_key,
    const uint8_t *iv,
    const uint8_t *plaintext,
    uint32_t pt_len,
    uint8_t *ciphertext)
{
    uint8_t counter_block[16];
    uint8_t keystream_block[16];

    for (int i = 0; i < 12; i++) counter_block[i] = iv[i];
    counter_block[12] = 0x00;
    counter_block[13] = 0x00;
    counter_block[14] = 0x00;
    counter_block[15] = 0x01;

    inc32(counter_block);

    uint32_t bytes_processed = 0;
    while (bytes_processed < pt_len) {
        for (int i = 0; i < 16; i++) keystream_block[i] = counter_block[i];
        AES_EncryptBlock(keystream_block, expanded_key);
        for (int i = 0; i < 16 && bytes_processed < pt_len; i++, bytes_processed++) {
            ciphertext[bytes_processed] = plaintext[bytes_processed] ^ keystream_block[i];
        }
        inc32(counter_block);
    }
}
// The encryption block for AES_GCM, which includes encryption and authentication of Ciphertext, Length block, and authentication of AAD(magic, sender_id, seq, etc.)
// 'tag' is the authentication tag that is obtained by XORing the final GHASH accumulator and the AES encryption of J0 (Which is (iv || 0x00000001)
//aad is an array that includes everything in the header, that has to be authenticated, but not encrypted. To be defined in main.c before calling this encryption block
void aes_gcm_encrypt(
    const uint8_t *key,
    const uint8_t *iv,
    const uint8_t *aad,
    uint32_t aad_len,
    const uint8_t *plaintext,
    uint32_t pt_len,
    uint8_t *ciphertext,
    uint8_t *tag
) {
    uint8_t expanded_key[AES128_EXPANDED_KEY_SIZE];
    uint8_t H[16] = {0};
    uint8_t J0[16];
    uint8_t tag_mask[16];
    uint8_t X[16] = {0};
    uint8_t temp_block[16] = {0};

    KeyExpansion(key, expanded_key);
    AES_EncryptBlock(H, expanded_key);

    for (int i = 0; i < 12; i++) J0[i] = iv[i];
    J0[12] = 0x00; J0[13] = 0x00; J0[14] = 0x00; J0[15] = 0x01;

    for (int i = 0; i < 16; i++) tag_mask[i] = J0[i];
    AES_EncryptBlock(tag_mask, expanded_key);

    uint32_t aad_pos = 0;
    while (aad_pos < aad_len) {
        for (int i = 0; i < 16; i++) temp_block[i] = 0;
        for (int i = 0; i < 16 && aad_pos < aad_len; i++, aad_pos++) {
            temp_block[i] = aad[aad_pos];
        }
        xor_block(X, temp_block);
        gf_mult(X, H, X);
    }

    gcm_encrypt_ctr(expanded_key, iv, plaintext, pt_len, ciphertext);

    uint32_t ct_pos = 0;
    while (ct_pos < pt_len) {
        for (int i = 0; i < 16; i++) temp_block[i] = 0;
        for (int i = 0; i < 16 && ct_pos < pt_len; i++, ct_pos++) {
            temp_block[i] = ciphertext[ct_pos];
        }
        xor_block(X, temp_block);
        gf_mult(X, H, X);
    }

    for (int i = 0; i < 16; i++) temp_block[i] = 0;
    uint64_t aad_bits = (uint64_t)aad_len * 8;
    uint64_t pt_bits  = (uint64_t)pt_len  * 8;

    for (int i = 7; i >= 0; i--) { temp_block[i] = (uint8_t)(aad_bits & 0xFF); aad_bits >>= 8; }
    for (int i = 15; i >= 8; i--) { temp_block[i] = (uint8_t)(pt_bits  & 0xFF); pt_bits  >>= 8; }

    xor_block(X, temp_block);
    gf_mult(X, H, X);

    for (int i = 0; i < 16; i++) tag[i] = X[i] ^ tag_mask[i];
}

//Decryption block, which also includes recalculating of Authentication tag to make sure tampering hasn't taken place
int aes_gcm_decrypt(
    const uint8_t *key,    const uint8_t *iv,
    const uint8_t *aad,    uint32_t aad_len,
    const uint8_t *ciphertext, uint32_t ct_len,
    const uint8_t *tag,    uint8_t *plaintext)
{
    uint8_t computed_tag[16];
    aes_gcm_encrypt(key, iv, aad, aad_len,
                    ciphertext, ct_len,
                    plaintext, computed_tag);

    uint8_t diff = 0;
    for (int i = 0; i < 16; i++) diff |= computed_tag[i] ^ tag[i];
    return (diff == 0) ? 0 : -1;
}
