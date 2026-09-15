#ifndef KASM_RELOCATE_H
#define KASM_RELOCATE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static int relocate(unsigned char *memory, size_t size, const size_t *patches, size_t count) {
    for (size_t i = 0; i < count; i++) {
        size_t at = patches[i];
        if (at > size || size - at < 8)
            return 0;
        uint64_t offset = 0;
        for (int j = 0; j < 8; j++)
            offset |= (uint64_t)memory[at + (size_t)j] << (8 * j);
        if (offset >= size) {
            fprintf(stderr, "relocation target outside code\n");
            return 0;
        }
        uint64_t address = (uint64_t)(uintptr_t)memory + offset;
        for (int j = 0; j < 8; j++) {
            memory[at + (size_t)j] = (unsigned char)(address & 255);
            address >>= 8;
        }
    }
    return 1;
}

#endif // KASM_RELOCATE_H