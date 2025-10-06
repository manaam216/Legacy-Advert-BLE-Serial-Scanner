# Legacy Advert BLE Serial Scanner

A simple **Bluetooth Low Energy (BLE)** scanner built on **Zephyr RTOS** and **Nordic’s nRF Connect SDK (NCS)**.  
This application runs on the **nRF52840 Development Kit (DK)** and listens for **legacy advertising packets** on the **1M PHY** channel.

It prints detailed information including advertiser address, RSSI, advertisement type, and parsed AD structures such as device name, manufacturer data, and UUIDs.

---

## Features

- Initializes the Zephyr Bluetooth stack and starts scanning on PHY 1M  
- Uses **passive scanning** (no scan requests) by default  
- Displays:
  - Advertiser address  
  - RSSI (signal strength)  
  - Advertisement type  
  - Device name (complete & shortened)  
  - Manufacturer-specific data  
  - UUID16 and UUID128 fields  
- Includes **CRC utility** and **Crypto utility** components for packet integrity and data processing  

---

## Requirements

- [nRF Connect SDK (NCS)](https://developer.nordicsemi.com/nRF_Connect_SDK/)
- Zephyr RTOS (included with NCS)
- **nRF52840 DK** or compatible BLE hardware
- [West](https://docs.zephyrproject.org/latest/develop/west/index.html) build tool

---

## Project Structure

``` bash

LEGACY_BLE_SERIAL_SCANNER/
├── components/
│ └── crc_compute/ # CRC computation module
│ ├── include/ # CRC function headers
│ ├── src/ # CRC implementation source
│ └── CMakeLists.txt
│
├── include/ # Global header files
│ ├── crypto_util.h
│ ├── packet.h
│ ├── scanner.h
│ └── worker.h
│
├── src/ # Application source files
│ ├── crypto_util.c
│ ├── main.c
│ ├── packet.c
│ ├── scanner.c
│ └── worker.c
│
├── nrf52840dk_nrf52840.overlay # Board device tree overlay
├── prj.conf # Zephyr configuration file
├── CMakeLists.txt # Main build configuration
├── .gitignore
└── README.md
```
---

## Building

Run the following command from the project root:

```bash
west build --build-dir build/ \
    . \
    --pristine --board nrf52840dk/nrf52840 --no-sysbuild \
    -- -DCONF_FILE="prj.conf" \
       -DDTC_OVERLAY_FILE=nrf52840dk_nrf52840.overlay

```

This will compile the application for the nRF52840 DK.
If using another target board, replace the board name accordingly.

## Running

1. Flash the application to the board with `west flash`.

2. Open a serial terminal (e.g., nRF Connect for Desktop, JLinkRTTViewer, or PuTTY).

3. Reset the board.

4. The console will display messages such as:

```sh
Starting BLE legacy-advertisement scanner
Bluetooth initialized
Scanning started (legacy adv on 1M PHY)
ADV from xx:xx:xx:xx:xx:xx (random) RSSI -48 dBm adv_type 0x00 props 0x0010 len 18
    Name: Zephyr_Device
    Manufacturer data (4 bytes): 01 02 03 04
```

## Configuration Notes
- The scan type is set to **passive**. To switch to active scanning (send scan requests and receive scan responses), change `BT_LE_SCAN_TYPE_PASSIVE` to `BT_LE_SCAN_TYPE_ACTIVE` in `scanner.c`.  
- The scan interval and window are configured for continuous scanning. Adjust values in `scan_param` to change timing.  
- Logging is done with `printk`. For more advanced debugging, enable Zephyr logging (`CONFIG_LOG=y`).  

---

## References
- [Zephyr Bluetooth API](https://docs.zephyrproject.org/latest/connectivity/bluetooth/index.html)  
- [nRF Connect SDK Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/)  
- [Zephyr Scan & Advertise Sample](https://docs.zephyrproject.org/latest/samples/bluetooth/scan_adv/README.html)  
