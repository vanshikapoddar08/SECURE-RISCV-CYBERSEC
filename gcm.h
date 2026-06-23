#ifndef GCM_H
#define GCM_H

#include <stdint.h>

void aes_gcm_encrypt(
    const uint8_t *key,
    const uint8_t *iv,
    const uint8_t *aad,
    uint32_t aad_len,
    const uint8_t *plaintext,
    uint32_t pt_len,
    uint8_t *ciphertext,
    uint8_t *tag
);

int aes_gcm_decrypt(
    const uint8_t *key,
    const uint8_t *iv,
    const uint8_t *aad,
    uint32_t aad_len,
    const uint8_t *ciphertext,
    uint32_t ct_len,
    const uint8_t *tag,
    uint8_t *plaintext
);


#endif