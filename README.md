# Legacy-Advert-BLE-Serial-Scanner

This project demonstrates how to implement a simple Bluetooth Low Energy (BLE) scanner on Zephyr RTOS and Nordic’s nRF Connect SDK (NCS).  
The application runs on the **nRF52840 Development Kit** and is configured to receive **legacy advertising packets** on the 1M PHY.  

It prints the advertiser address, RSSI, advertisement type, and parses common AD structures such as device name, manufacturer data, and UUIDs.

---

## Features
- Initializes the Zephyr Bluetooth stack and starts scanning on PHY 1M  
- Uses **passive scanning** (no scan requests) by default  
- Prints advertiser address and signal strength (RSSI)  
- Parses and displays common AD fields:
  - Complete and shortened local name
  - Manufacturer specific data
  - UUID16 and UUID128 fields
- Notifies when a scan response is received
- Provides a scan timeout callback

---

## Requirements
- [nRF Connect SDK (NCS)](https://developer.nordicsemi.com/nRF_Connect_SDK/)  
- Zephyr RTOS (included with NCS)  
- **nRF52840 DK** or another board with BLE support  
- West build toolchain  

---

## Project Structure
- `main.c`: Application source code with BLE observer logic  
- `prj.conf`: Application configuration file (Bluetooth, observer role, logging)  
- `CMakeLists.txt` and `west.yml`: Standard Zephyr build system files  
- `nrf52840dk_nrf52840.overlay`: Optional device tree overlay (board specific)  

---

## Building
Run the following command from the project root:

```sh
west build --build-dir {path_to_target_build_dir} \
    {path_to_prj_dir} \
    --pristine --board nrf52840dk/nrf52840 --no-sysbuild \
    -- -DCONF_FILE="prj.conf" \
       -DDTC_OVERLAY_FILE=nrf52840dk_nrf52840.overlay
```

This builds the application for the nRF52840 DK. Adjust the board name if using a different target.

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
- The scan type is set to **passive**. To switch to active scanning (send scan requests and receive scan responses), change `BT_LE_SCAN_TYPE_PASSIVE` to `BT_LE_SCAN_TYPE_ACTIVE` in `main.c`.  
- The scan interval and window are configured for continuous scanning. Adjust values in `scan_param` to change timing.  
- Logging is done with `printk`. For more advanced debugging, enable Zephyr logging (`CONFIG_LOG=y`).  

---

## References
- [Zephyr Bluetooth API](https://docs.zephyrproject.org/latest/connectivity/bluetooth/index.html)  
- [nRF Connect SDK Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/)  
- [Zephyr Scan & Advertise Sample](https://docs.zephyrproject.org/latest/samples/bluetooth/scan_adv/README.html)  
