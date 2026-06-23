#ifndef AES_H
#define AES_H

#include <stdint.h>

#define AES_BLOCK_SIZE 16
#define AES128_KEY_SIZE 16
#define AES128_ROUNDS 10
#define AES128_EXPANDED_KEY_SIZE 176


void KeyExpansion(const uint8_t *key, uint8_t *expanded_key);

void AES_EncryptBlock(uint8_t *state, const uint8_t *expanded_key);

#endif
