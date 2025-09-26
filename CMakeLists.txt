cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(legacy_ble_serial_scanner)

add_subdirectory(components/crc_compute)

target_sources(app PRIVATE src/main.c)
