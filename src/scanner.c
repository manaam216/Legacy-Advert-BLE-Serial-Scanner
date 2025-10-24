#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/net_buf.h>
#include <string.h>
#include <crc_compute.h>
#include "scanner.h"
#include "worker.h"
#include "packet.h"

LOG_MODULE_REGISTER(scanner, LOG_LEVEL_INF);

static void device_found(const bt_addr_le_t *addr, int8_t rssi,
                         uint8_t adv_type, struct net_buf_simple *ad)
{
    struct net_buf_simple_state state;
    uint8_t ad_type, ad_len;
    uint8_t *ad_ptr;

    net_buf_simple_save(ad, &state);

    while (ad->len > 0) {
        uint8_t len = net_buf_simple_pull_u8(ad);
        if (len == 0) {
            break;
        }

        ad_type = net_buf_simple_pull_u8(ad);
        ad_len  = len - 1;
        ad_ptr  = ad->data;

        if (ad_len == PAYLOAD_FRAME_LENGTH && ad_type == BT_DATA_MANUFACTURER_DATA) {
            const uint8_t *raw = ad_ptr;

            uint8_t crc_rx   = raw[ad_len - 1];
            uint8_t crc_calc = compute_crc(CRC_TYPE_8, (uint8_t *)raw, ad_len - 1);

            if (crc_rx != crc_calc) {
                LOG_WRN("CRC mismatch from %p", addr);
            } else {
                struct decrypt_job job;
                job.flags_raw = raw[2];
                memcpy(job.encrypted, &raw[3], ENCRYPTED_DATA_SIZE_BYTES);
                job.rssi = rssi;
                memcpy(&job.addr, addr, sizeof(bt_addr_le_t));

                if (enqueue_job(&job) != 0) {
                    LOG_WRN("Decrypt queue full, dropping frame");
                }
            }
        }
        net_buf_simple_pull(ad, ad_len);
    }

    net_buf_simple_restore(ad, &state);
}

int scanner_start(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

    LOG_INF("Bluetooth initialized");

    static const struct bt_le_scan_param scan_param = {
        .type     = BT_LE_SCAN_TYPE_PASSIVE,
        .options  = BT_LE_SCAN_OPT_NONE,
        .interval = 0x0010,
        .window   = 0x0010,
    };

    err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        LOG_ERR("Scanning start failed (err %d)", err);
        return err;
    }

    LOG_INF("Scanning started");
    return 0;
}
