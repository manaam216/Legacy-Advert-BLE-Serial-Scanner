/* main.c - Zephyr BLE legacy-advertisement observer example */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/net_buf.h>

/* helper: print bt address */
static void print_addr(const bt_addr_le_t *addr)
{
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    printk("%s", addr_str);
}

/* callback used by bt_data_parse to handle AD structures */
static bool ad_data_cb(struct bt_data *data, void *user_data)
{
    switch (data->type) {
    case BT_DATA_NAME_COMPLETE:
    case BT_DATA_NAME_SHORTENED: {
        char name[32] = {0};
        size_t len = MIN(data->data_len, sizeof(name) - 1);
        memcpy(name, data->data, len);
        printk("    Name: %s\n", name);
        break;
    }
    case BT_DATA_MANUFACTURER_DATA:
        printk("    Manufacturer data (%u bytes):", data->data_len);
        for (size_t i = 0; i < data->data_len; i++) {
            printk(" %02x", data->data[i]);
        }
        printk("\n");
        break;
    case BT_DATA_UUID16_SOME:
    case BT_DATA_UUID16_ALL:
        printk("    UUID16 adv field (len=%u)\n", data->data_len);
        break;
    case BT_DATA_UUID128_SOME:
    case BT_DATA_UUID128_ALL:
        printk("    UUID128 adv field (len=%u)\n", data->data_len);
        break;
    default:
        /* other AD types can be handled here */
        break;
    }
    /* return true to continue parsing remaining AD fields */
    return true;
}

/* scan receive callback: gets called for each advertisement packet */
static void scan_recv_cb(const struct bt_le_scan_recv_info *info,
                         struct net_buf_simple *buf)
{
    /* Print address and RSSI */
    printk("ADV from ");
    print_addr(info->addr);
    printk("  RSSI %d dBm  adv_type 0x%02x  props 0x%04x len %u\n",
           info->rssi, info->adv_type, info->adv_props, buf->len);

    /* Parse advertising data (buf contains advertising payload as net_buf_simple) */
    bt_data_parse(buf, ad_data_cb, NULL);

    /* If this was a scan response, it will be indicated (adv_type/adv_props). */
    if (info->adv_props & BT_GAP_ADV_PROP_SCAN_RESPONSE) {
        printk("  (this report contains a scan response)\n");
    }
}

/* optional timeout callback when scan param timeout expires */
static void scan_timeout_cb(void)
{
    printk("Scan timeout reached\n");
}

/* register scan callbacks (listener struct) */
static struct bt_le_scan_cb scan_cb = {
    .recv = scan_recv_cb,
    .timeout = scan_timeout_cb,
};

int main(void)
{
    int err;

    printk("Starting BLE legacy-advertisement scanner\n");

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    printk("Bluetooth initialized\n");

    /* Register scan callback listener */
    bt_le_scan_cb_register(&scan_cb);

    /* Choose scan parameters for legacy advertising (1M PHY). Use passive or active scan.
     * Passive scanning receives advertising PDUs only (no scan-requests). Active will
     * also issue scan requests to retrieve scan response PDUs when supported.
     */
    static const struct bt_le_scan_param scan_param = BT_LE_SCAN_PARAM_INIT(
        BT_LE_SCAN_TYPE_PASSIVE, /* active scanning to also receive scan responses */
        BT_LE_SCAN_OPT_NONE,    /* options, e.g. filter-duplicate if supported */
        0x0010,                 /* interval (N * 0.625 ms) -> 10 * 0.625 = 6.25 ms */
        0x0010                  /* window (same as interval -> continuous) */
    );

    err = bt_le_scan_start(&scan_param, NULL /* legacy API accepts callback here on older versions; we registered cb above */);
    if (err) {
        printk("Failed to start scanning (err %d)\n", err);
        return -1;
    }

    printk("Scanning started (legacy adv on 1M PHY)\n");

    return 0;
}
