#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <string.h>
#include "worker.h"
#include "crypto_util.h"
#include "packet.h"

#define MAX_JOBS 300
K_MSGQ_DEFINE(decrypt_msgq, sizeof(struct decrypt_job), MAX_JOBS, 4);



static void decrypt_worker(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    struct decrypt_job job;
    uint8_t decrypted[ENCRYPTED_DATA_SIZE_BYTES];

    while (1) {
        if (k_msgq_get(&decrypt_msgq, &job, K_FOREVER) == 0) {
            ble_packet_flags_t flags = {.raw = job.flags_raw};

            int dec_len = -1;
            if (flags.bits.encrypt_status == 0) {
                dec_len = decrypt_payload(job.encrypted, ENCRYPTED_DATA_SIZE_BYTES, decrypted);
            } else {
                memcpy(decrypted, job.encrypted, ENCRYPTED_DATA_SIZE_BYTES);
                dec_len = ENCRYPTED_DATA_SIZE_BYTES;
            }

            char addr_str[BT_ADDR_LE_STR_LEN];
            bt_addr_le_to_str(&job.addr, addr_str, sizeof(addr_str));

            if (dec_len == ENCRYPTED_DATA_SIZE_BYTES) {
                ble_pkt_encrypted_data_t packet;
                memcpy(packet.data_bytes, decrypted, ENCRYPTED_DATA_SIZE_BYTES);

                printk("\nDecoded frame from %s (RSSI %d)\n", addr_str, job.rssi);
                print_decoded_packet(&packet, &flags);
            } else {
                printk("Decryption failed (addr %s, rssi %d)\n", addr_str, job.rssi);
            }
        }
    }
}

int enqueue_job(const struct decrypt_job *job)
{
    return k_msgq_put(&decrypt_msgq, job, K_NO_WAIT);
}

K_THREAD_DEFINE(decrypt_tid, 4096, decrypt_worker, NULL, NULL, NULL,
                        5, 0, 0);

void start_worker(void)
{

}
