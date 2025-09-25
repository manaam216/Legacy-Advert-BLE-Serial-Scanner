#ifndef WORKER_H
#define WORKER_H

#include <zephyr/bluetooth/bluetooth.h>

#define ENCRYPTED_DATA_SIZE_BYTES   16

struct decrypt_job {
    uint8_t encrypted[ENCRYPTED_DATA_SIZE_BYTES];
    uint8_t flags_raw;
    int8_t rssi;
    bt_addr_le_t addr;
};

/* API */
int enqueue_job(const struct decrypt_job *job);
void start_worker(void);

#endif /* WORKER_H */
