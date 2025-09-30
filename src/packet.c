#include <zephyr/sys/printk.h>
#include "packet.h"

void print_decoded_packet(const ble_pkt_encrypted_data_t *pkt,
                          const ble_packet_flags_t *flags)
{
    printk("Device SRC ID: %02X%02X%02X\n",
           pkt->fields.src_id[2], pkt->fields.src_id[1], pkt->fields.src_id[0]);

    printk("Network ID: %02X%02X\n",
           pkt->fields.nwk_id[1], pkt->fields.nwk_id[0]);

    printk("FW Version: %u\n", pkt->fields.fw_version);

    printk("Sensor Type: %02X%02X\n",
           pkt->fields.sensor_type[1], pkt->fields.sensor_type[0]);

    printk("Event Counter (LSB only): %u\n", flags->bits.event_counter_lsb);
    printk("Payload Len: %u\n", flags->bits.payload_length);
    printk("Encrypt Status: %u\n", flags->bits.encrypt_status);
    printk("Power Status: %u\n", flags->bits.self_external_power);
    printk("Flags %u\n", flags->raw);

    printk("Payload: ");
    for (size_t i = 0; i < sizeof(pkt->fields.payload); ++i) {
        printk("%02X ", pkt->fields.payload[i]);
    }
    printk("\n");
}
