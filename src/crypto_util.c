#include <zephyr/device.h>
#include <zephyr/crypto/crypto.h>
#include <zephyr/sys/printk.h>
#include "crypto_util.h"

#define AES_KEY_SIZE 16

static uint8_t ecb_key[AES_KEY_SIZE] = {
    0x00,0x01,0x02,0x03,
    0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,
    0x0C,0x0D,0x0E,0x0F
};

int decrypt_payload(uint8_t *input, size_t len, uint8_t *output)
{
    const struct device *dev_crypto =
        device_get_binding(CONFIG_CRYPTO_MBEDTLS_SHIM_DRV_NAME);
    if (!dev_crypto || !device_is_ready(dev_crypto)) {
        printk("Crypto device not ready\n");
        return -1;
    }

    struct cipher_ctx ctx = {0};
    ctx.keylen = AES_KEY_SIZE;
    ctx.key.bit_stream = ecb_key;

    if (cipher_begin_session(dev_crypto, &ctx,
                             CRYPTO_CIPHER_ALGO_AES,
                             CRYPTO_CIPHER_MODE_ECB,
                             CRYPTO_CIPHER_OP_DECRYPT)) {
        printk("Failed to start AES-ECB session\n");
        return -1;
    }

    struct cipher_pkt pkt = {
        .in_buf = input,
        .in_len = len,
        .out_buf = output,
        .out_buf_max = len,
    };

    int ret = cipher_block_op(&ctx, &pkt);
    cipher_free_session(dev_crypto, &ctx);

    return ret ? -1 : pkt.out_len;
}
