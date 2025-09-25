cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(legacy_ble_serial_scanner)

add_subdirectory(components/crc_compute)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(app PRIVATE
    src/main.c
    src/scanner.c
    src/worker.c
    src/crypto_util.c
    src/packet.c
)
