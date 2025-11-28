#include <zephyr/sys/printk.h>
#include <stdint.h>
#include <string.h>

#include "packet.h"

void print_decoded_packet(const ble_pkt_encrypted_data_t *pkt,
                          const ble_packet_flags_t *flags)
{
    if (!pkt || !flags) {
        printk("ERR: NULL packet\n");
        return;
    }

    printk("SRC ID: %02X%02X%02X\n",
           pkt->fields.src_id[2],
           pkt->fields.src_id[1],
           pkt->fields.src_id[0]);

    printk("NWK ID: %02X%02X\n",
           pkt->fields.nwk_id[1],
           pkt->fields.nwk_id[0]);

    printk("Sensor Type: 0x%02X\n", pkt->fields.sensor_type);

    printk("Flags: raw=%02X enc=%u ext=%u ec_lsb=%u pl_len=%u\n",
           flags->raw,
           flags->bits.encrypt_status,
           flags->bits.self_external_power,
           flags->bits.event_counter_lsb,
           flags->bits.payload_length);

    /* NEW PACKET FORMAT */
    const uint8_t *p = pkt->fields.payload;

    uint32_t event_counter = ((uint32_t)p[0]) |
                             ((uint32_t)p[1] << 8) |
                             ((uint32_t)p[2] << 16);

    uint8_t event_type = p[3];

    printk("Event Counter: %u\n", event_counter);
    printk("Event Type: %u\n", event_type);

    /* 4–9 bytes = type-specific payload */
    switch (event_type)
    {
        /* TYPE 0 (BUTTON_PRESS) and TYPE 3 (POLARITY)—both carry accelerometer */
        case EVENT_TYPE_BUTTON_PRESS:
        case EVENT_TYPE_BUTTON_ON:
        case EVENT_TYPE_BUTTON_OFF:
        {
            int16_t ax = (int16_t)((uint16_t)p[4] | ((uint16_t)p[5] << 8));
            int16_t ay = (int16_t)((uint16_t)p[6] | ((uint16_t)p[7] << 8));
            int16_t az = (int16_t)((uint16_t)p[8] | ((uint16_t)p[9] << 8));

            printk("Accel: X=%d Y=%d Z=%d\n", ax, ay, az);
            break;
        }

        /* TYPE 2 (VIBRATION) — random/sample bytes */
        case EVENT_TYPE_VIBRATION:
        case EVENT_TYPE_LEAK_DETECTED:
        {
            printk("Sample Bytes: %02X %02X %02X %02X %02X %02X\n",
                    p[4], p[5], p[6], p[7], p[8], p[9]);
            break;
        }

        case EVENT_TYPE_EXTERNAL_POWER:
            printk("External Power Event\n");
            break;

        default:
            printk("Unknown Event Type: %u\n", event_type);
            break;
    }
}
