#include "kasm/emit.h"

#include <stdlib.h>

int byte_push(Bytes *bytes, uint8_t value) {
    if (!bytes)
        return 0;

    if (bytes->count == bytes->capacity) {
        size_t new_capacity;

        if (bytes->capacity == 0) {
            new_capacity = KASM_BYTES_INITIAL_CAPACITY;
        }
        else {
            if (bytes->capacity > SIZE_MAX / 2)
                return 0;
            new_capacity = bytes->capacity * 2;
        }

        uint8_t *new_data = realloc(
            bytes->data, new_capacity * sizeof(*new_data));
        if (!new_data)
            return 0;

        bytes->data = new_data;
        bytes->capacity = new_capacity;
    }

    bytes->data[bytes->count++] = value;
    return 1;
}

void bytes_free(Bytes *bytes) {
    if (!bytes)
        return;

    free(bytes->data);
    bytes->data = NULL;
    bytes->count = 0;
    bytes->capacity = 0;
}
