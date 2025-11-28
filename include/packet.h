#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PAYLOAD_FRAME_LENGTH 20
#define ENCRYPTED_DATA_SIZE_BYTES   16
#define EVENT_COUNTER_NUM_BYTES     3
#define SRC_ID_NUM_BYTES            3
#define SENSOR_TYPE_NUM_BYTES       2
#define PAYLOAD_MAX_NUM_BYTES       10
#define NETWORK_ID_NUM_BYTES        2

typedef enum
{
	EVENT_TYPE_BUTTON_PRESS = 0,
	EVENT_TYPE_VIBRATION,
	EVENT_TYPE_BUTTON_ON,
	EVENT_TYPE_BUTTON_OFF,
	EVENT_TYPE_LEAK_DETECTED,
	EVENT_TYPE_EXTERNAL_POWER = 255,
}event_type_t;


typedef union {
    struct 
    {
        uint8_t src_id[SRC_ID_NUM_BYTES];
        uint8_t nwk_id[NETWORK_ID_NUM_BYTES];
        uint8_t sensor_type;
        uint8_t payload[PAYLOAD_MAX_NUM_BYTES];
    } fields;
    uint8_t data_bytes[ENCRYPTED_DATA_SIZE_BYTES];
} ble_pkt_encrypted_data_t;

typedef union 
{
    struct {
        uint8_t encrypt_status      : 1;
        uint8_t self_external_power : 1; 
        uint8_t event_counter_lsb   : 2; 
        uint8_t payload_length      : 4; 
    } bits;
    uint8_t raw; // direct access to full byte
} ble_packet_flags_t;
void print_decoded_packet(const ble_pkt_encrypted_data_t *pkt,
                          const ble_packet_flags_t *flags);

#endif /* PACKET_H */
