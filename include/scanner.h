#ifndef SCANNER_H
#define SCANNER_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

/* Initialize and start BLE scanning */
int scanner_start(void);

#endif /* SCANNER_H */
