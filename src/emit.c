
#include "kasm/emit.h"

/*
 * Converts a value to little-endian byte order and appends it to the given Bytes structure.
 * @param bytes The Bytes structure to append the little-endian bytes to.
 * @param value The value to convert to little-endian byte order.
 * @param width The number of bytes to write in little-endian order.
*/
int little_endian(Bytes *bytes, uint64_t value, size_t width) {
     if (!bytes || width > sizeof(value))
        return 0;

    for (size_t i = 0; i < width; i++) {
        uint8_t byte = (uint8_t)(value >> (i * 8));

        if (!byte_push(bytes, byte))
            return 0;
    }

    return 1;
}