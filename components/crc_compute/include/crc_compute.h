#ifndef __CRC_COMPUTE__
#define __CRC_COMPUTE__

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CRC_TYPE_8,
    CRC_TYPE_16,
    CRC_TYPE_32
} crc_type_t;

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
 */
uint32_t compute_crc(crc_type_t type, const uint8_t *data, size_t len);

#endif // __CRC_COMPUTE__