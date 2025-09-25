#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "scanner.h"
#include "worker.h"

int main(void)
{
    printk("Starting BLE scanner (modularized)...\n");

    start_worker();

    if (scanner_start() != 0) {
        printk("Scanner failed\n");
    }

    return 0;
}
