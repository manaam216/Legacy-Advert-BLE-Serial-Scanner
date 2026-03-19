#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#include "scanner.h"

LOG_MODULE_REGISTER(scanner, LOG_LEVEL_INF);

static void device_found(const bt_addr_le_t *addr,
                         int8_t rssi,
                         uint8_t adv_type,
                         struct net_buf_simple *ad)
{
    char name[32] = {0};
    const uint8_t *mfg = NULL;
    uint8_t mfg_len = 0;

    ARG_UNUSED(adv_type);

    while (ad->len > 1U) {
        uint8_t len = net_buf_simple_pull_u8(ad);

        if (len == 0U || ad->len < len) {
            break;
        }

        uint8_t type = net_buf_simple_pull_u8(ad);
        uint8_t data_len = len - 1U;
        uint8_t *data = ad->data;

        if ((type == BT_DATA_NAME_COMPLETE || type == BT_DATA_NAME_SHORTENED) &&
            data_len > 0U) {
            uint8_t copy_len = (data_len < sizeof(name) - 1U) ? data_len : (sizeof(name) - 1U);
            memcpy(name, data, copy_len);
            name[copy_len] = '\0';
        } else if (type == BT_DATA_MANUFACTURER_DATA) {
            mfg = data;
            mfg_len = data_len;
        }

        net_buf_simple_pull(ad, data_len);
    }

    printk("MAC:%02X:%02X:%02X:%02X:%02X:%02X RSSI:%d Name:%s MFG:",
           addr->a.val[5], addr->a.val[4], addr->a.val[3],
           addr->a.val[2], addr->a.val[1], addr->a.val[0],
           rssi,
           name[0] ? name : "N/A");

    if (mfg && mfg_len) {
        for (uint8_t i = 0; i < mfg_len; i++) {
            printk("%02X", mfg[i]);
        }
    } else {
        printk("N/A");
    }

    printk("\n");
}

int scanner_start(void)
{
    int err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

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

    printk("Scanning started\n");
    return 0;
}