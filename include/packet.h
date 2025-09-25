#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PAYLOAD_FRAME_LENGTH 18
#define ENCRYPTED_DATA_SIZE_BYTES 16

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

void print_decoded_packet(const ble_pkt_encrypted_data_t *pkt,
                          const ble_packet_flags_t *flags);

#endif /* PACKET_H */
