/*
 * BLE scanner: decode packets (format from script #1) using queued
 * decryption architecture (script #2).
 *
 * - Expects Manufacturer data layout:
 *   [0]     : flags
 *   [1..16] : encrypted payload (16 bytes)
 *   [17]    : CRC8 over bytes [0..16]
 *
 * - Worker dequeues decrypt jobs, decrypts (or passes through if not encrypted),
 *   decodes fields and prints them.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/crypto/crypto.h>
#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <crc_compute.h>

LOG_MODULE_REGISTER(we_ble_scanner, LOG_LEVEL_INF);

/* --- Frame layout (from your first script) --- */
#define PAYLOAD_FRAME_LENGTH        18  /* 1 flag + 16 encrypted + 1 CRC */
#define ENCRYPTED_DATA_SIZE_BYTES   16
#define AES_KEY_SIZE                16

/* --- Decryption queue setup --- */
#define MAX_JOBS 300

struct decrypt_job {
    uint8_t encrypted[ENCRYPTED_DATA_SIZE_BYTES];
    uint8_t flags_raw;
    int8_t rssi;
    bt_addr_le_t addr;
};

/* Queue with MAX_JOBS slots, each holding one decrypt_job */
K_MSGQ_DEFINE(decrypt_msgq, sizeof(struct decrypt_job), MAX_JOBS, 4);

/* --- Crypto key (example) --- */
static uint8_t ecb_key[AES_KEY_SIZE] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F
};

/* --- Packet structs from script #1 --- */
typedef union {
    struct {
        uint8_t src_id[3];
        uint8_t nwk_id[2];
        uint8_t fw_version;
        uint8_t sensor_type[2];
        uint8_t payload[8];
    } fields;
    uint8_t data_bytes[ENCRYPTED_DATA_SIZE_BYTES];
} ble_pkt_encrypted_data_t;

typedef union {
    struct {
        uint8_t encrypt_status      : 1;
        uint8_t self_external_power : 1;
        uint8_t event_counter_lsb   : 2;
        uint8_t payload_length      : 4;
    } bits;
    uint8_t raw;
} ble_packet_flags_t;

/* --- Crypto helper (AES-ECB decrypt) --- */
static int decrypt_payload(uint8_t *input, size_t len, uint8_t *output)
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

/* --- Worker thread: dequeue, decrypt, decode, print --- */
void decrypt_worker(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    struct decrypt_job job;
    uint8_t decrypted[ENCRYPTED_DATA_SIZE_BYTES];

    while (1) {
        /* Block until a job is available */
        if (k_msgq_get(&decrypt_msgq, &job, K_FOREVER) == 0) {
            ble_packet_flags_t flags;
            flags.raw = job.flags_raw;

            int dec_len = -1;
            if (flags.bits.encrypt_status == 0) {
                dec_len = decrypt_payload(job.encrypted, ENCRYPTED_DATA_SIZE_BYTES, decrypted);
            } else {
                /* Not encrypted: copy as-is */
                memcpy(decrypted, job.encrypted, ENCRYPTED_DATA_SIZE_BYTES);
                dec_len = ENCRYPTED_DATA_SIZE_BYTES;
            }

            char addr_str[BT_ADDR_LE_STR_LEN];
            bt_addr_le_to_str(&job.addr, addr_str, sizeof(addr_str));

            if (dec_len == ENCRYPTED_DATA_SIZE_BYTES) {
                ble_pkt_encrypted_data_t packet;
                memcpy(packet.data_bytes, decrypted, ENCRYPTED_DATA_SIZE_BYTES);

                printk("\nDecoded frame from %s (RSSI %d)\n", addr_str, job.rssi);
                printk("Device SRC ID: %02X%02X%02X\n",
                       packet.fields.src_id[2],
                       packet.fields.src_id[1],
                       packet.fields.src_id[0]);

                printk("Network ID: %02X%02X\n",
                       packet.fields.nwk_id[1],
                       packet.fields.nwk_id[0]);

                printk("FW Version: %u\n", packet.fields.fw_version);

                printk("Sensor Type: %02X%02X\n",
                       packet.fields.sensor_type[1],
                       packet.fields.sensor_type[0]);

                printk("Event Counter (LSB only): %u\n", flags.bits.event_counter_lsb);
                printk("Payload Len: %u\n", flags.bits.payload_length);
                printk("Encrypt Status: %u\n", flags.bits.encrypt_status);
                printk("Power Status: %u\n", flags.bits.self_external_power);

                /* Hex dump payload (8 bytes) */
                printk("Payload: ");
                for (size_t i = 0; i < sizeof(packet.fields.payload); ++i) {
                    printk("%02X ", packet.fields.payload[i]);
                }
                printk("\n");
            } else {
                printk("Decryption failed (addr %s, rssi %d)\n", addr_str, job.rssi);
            }
        }
    }
}

/* Create thread with enough stack and priority */
K_THREAD_DEFINE(decrypt_tid, 4096, decrypt_worker, NULL, NULL, NULL,
                5, 0, 0);

/* device_found: parse advertisement, handle manufacturer frames and enqueue decrypt jobs */
static void device_found(const bt_addr_le_t *addr, int8_t rssi,
                         uint8_t adv_type, struct net_buf_simple *ad)
{
    /* We will iterate the AD and find manufacturer data ourselves instead of relying on ad_parse_cb's user_data,
     * because we need address and rssi for the job. Simpler: parse manually to locate BT_DATA_MANUFACTURER_DATA */
    struct net_buf_simple_state state;
    uint8_t ad_type, ad_len;
    uint8_t *ad_ptr;

    net_buf_simple_save(ad, &state);

    while (ad->len > 0) {
        /* AD structure: length (1) + type (1) + data (length-1) */
        uint8_t len = net_buf_simple_pull_u8(ad);
        if (len == 0) {
            break;
        }

        ad_type = net_buf_simple_pull_u8(ad);
        ad_len = len - 1;
        ad_ptr = ad->data;

        if (ad_len > 0 && ad_type == BT_DATA_MANUFACTURER_DATA) {
            if (ad_len != PAYLOAD_FRAME_LENGTH) {
                LOG_WRN("Manuf data length %u unexpected", ad_len);
            } else {
                const uint8_t *raw = ad_ptr;

                uint8_t crc_rx = raw[ad_len - 1];
                uint8_t crc_calc = compute_crc(CRC_TYPE_8, (uint8_t *)raw, ad_len - 1);

                if (crc_rx != crc_calc) {
                    LOG_WRN("CRC mismatch from %p: rx=0x%02X calc=0x%02X", addr, crc_rx, crc_calc);
                } else {
                    struct decrypt_job job;
                    job.flags_raw = raw[0];
                    memcpy(job.encrypted, &raw[1], ENCRYPTED_DATA_SIZE_BYTES);
                    job.rssi = rssi;
                    memcpy(&job.addr, addr, sizeof(bt_addr_le_t));

                    if (k_msgq_put(&decrypt_msgq, &job, K_NO_WAIT) != 0) {
                        printk("Decrypt queue full, dropping frame from addr %p\n", addr);
                    } else {
                        /* queued */
                    }
                }
            }
        }

        /* advance pointer by ad_len */
        net_buf_simple_pull(ad, ad_len);
    }

    net_buf_simple_restore(ad, &state);
}

/* --- main: initialize BT and start scanning --- */
int main(void)
{
    int err;

    printk("Starting BLE scanner (queued decryption)...\n");

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return 0;
    }
    printk("Bluetooth initialized\n");

    /* Passive continuous scan parameters */
    static const struct bt_le_scan_param scan_param = {
        .type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
        .interval   = 0x0010,
        .window     = 0x0010,
    };

    err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        printk("Starting scanning failed (err %d)\n", err);
        return 0;
    }

    printk("Scanning started...\n");
    return 0;
}
