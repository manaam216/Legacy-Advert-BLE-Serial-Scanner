#ifndef CRYPTO_UTIL_H
#define CRYPTO_UTIL_H

#include <stddef.h>
#include <stdint.h>

int decrypt_payload(uint8_t *input, size_t len, uint8_t *output);

#endif /* CRYPTO_UTIL_H */
