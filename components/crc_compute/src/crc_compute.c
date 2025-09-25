#include "crc_compute.h"
#include <zephyr/sys/crc.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Compute CRC for a byte array with safety checks
 *
 * This function computes CRC-8, CRC-16 (CCITT), or CRC-32 (IEEE) depending
 * on the selected type. The CRC value is returned as a 32-bit integer,
 * but only the lower 8, 16, or 32 bits are valid depending on the CRC type.
 *
 * Input validation ensures NULL pointers, zero-length, and excessively large
 * buffers are rejected gracefully by returning 0.
 *
 * @param type   CRC type (CRC_TYPE_8, CRC_TYPE_16, CRC_TYPE_32)
 * @param data   Pointer to input buffer
 * @param len    Length of input buffer
 *
 * @return       CRC value as uint32_t (use lower 8/16/32 bits depending on type).
 *               Returns 0 on invalid input or unknown CRC type.
 *
 * @note CRC-16 uses CCITT polynomial (0x1021) with seed = 0xFFFF.
 *
 * @usage
 * @code
 *  #include "crc_compute.h"
 *
 *  uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04};
 *  uint32_t crc_val;
 *
 *  crc_val = compute_crc(CRC_TYPE_8, buffer, sizeof(buffer));
 *  printk("CRC-8 = 0x%02X\n", (uint8_t)crc_val);
 *
 *  crc_val = compute_crc(CRC_TYPE_16, buffer, sizeof(buffer));
 *  printk("CRC-16 = 0x%04X\n", (uint16_t)crc_val);
 *
 *  crc_val = compute_crc(CRC_TYPE_32, buffer, sizeof(buffer));
 *  printk("CRC-32 = 0x%08X\n", (uint32_t)crc_val);
 * @endcode
 */
uint32_t compute_crc(crc_type_t type, const uint8_t *data, size_t len)
{
    // ✅ Input checks
    if (data == NULL || len == 0) {
        return 0; // invalid input → return 0 as "error"
    }

    // Avoid very large lengths (e.g., corrupted pointers)
    if (len > 4096) {  
        return 0; // arbitrary safety limit
    }

    switch (type) {
    case CRC_TYPE_8:
        // poly = 0x07, init = 0x00, no reflection
        return (uint32_t)crc8(data, len, 0x07, 0x00, false);

    case CRC_TYPE_16:
        // CRC-16-CCITT with seed = 0xFFFF
        return (uint32_t)crc16_ccitt(0xFFFF, data, len);

    case CRC_TYPE_32:
        // IEEE 802.3 Ethernet polynomial (0x04C11DB7)
        return crc32_ieee(data, len);

    default:
        return 0; // unknown CRC type
    }
}
