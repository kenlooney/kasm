// Copyright 2026 Kenneth Looney
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



#include "elf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    ELF_HEADER_SIZE = 64,
    ELF_PROGRAM_HEADER_SIZE = 56,
    ELF_PAYLOAD_OFFSET = 0x1000
};

static const uint64_t ELF_BASE_ADDRESS = 0x400000;

static int number(Bytes *out, uint64_t value, unsigned width)
{
    for (unsigned i = 0; i < width; ++i)
    {
        if (!byte_push(out, (unsigned char)(value >> (8 * i))))
            return 0;
    }

    return 1;
}

static int append(Bytes *out, const void *data, size_t count)
{
    const unsigned char *bytes = data;

    for (size_t i = 0; i < count; ++i)
    {
        if (!byte_push(out, bytes[i]))
            return 0;
    }

    return 1;
}

int write_elf(const Bytes *code, size_t entry_offset, const char *path)
{
    if (code == NULL || path == NULL || path[0] == '\0' ||
        code->count == 0 ||
        code->data == NULL ||
        code->patch_count != 0 ||
        entry_offset >= code->count ||
        code->count > KASM_IMAGE_LIMIT - ELF_PAYLOAD_OFFSET)
    {
        fputs("invalid or unsupported ELF image\n", stderr);
        return 0;
    }

    const uint64_t file_size =
        (uint64_t)ELF_PAYLOAD_OFFSET + (uint64_t)code->count;

    const uint64_t entry_address =
        ELF_BASE_ADDRESS +
        (uint64_t)ELF_PAYLOAD_OFFSET +
        (uint64_t)entry_offset;

    Bytes file = {0};
    int okay = 0;

    /*
     * ELF identification:
     * 7F 'E' 'L' 'F' = ELF magic
     * 2 = 64-bit
     * 1 = little-endian
     * 1 = ELF version
     * Remaining identification bytes are zero.
     */
    const unsigned char identification[16] = {
        0x7F, 'E', 'L', 'F',
        2, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    if (!append(&file, identification, sizeof identification))
        goto done;

    /* 64-byte ELF header. */
    if (!number(&file, 2, 2) ||                         /* ET_EXEC */
        !number(&file, 0x3E, 2) ||                      /* x86-64 */
        !number(&file, 1, 4) ||                         /* ELF version */
        !number(&file, entry_address, 8) ||             /* entry address */
        !number(&file, ELF_HEADER_SIZE, 8) ||           /* program table offset */
        !number(&file, 0, 8) ||                         /* no section table */
        !number(&file, 0, 4) ||                         /* processor flags */
        !number(&file, ELF_HEADER_SIZE, 2) ||
        !number(&file, ELF_PROGRAM_HEADER_SIZE, 2) ||
        !number(&file, 1, 2) ||                         /* one program header */
        !number(&file, 0, 2) ||                         /* no section records */
        !number(&file, 0, 2) ||
        !number(&file, 0, 2))
    {
        goto done;
    }

    /*
     * One PT_LOAD program header mapping the entire file as readable and
     * executable, beginning at ELF_BASE_ADDRESS.
     */
    if (!number(&file, 1, 4) ||                         /* PT_LOAD */
        !number(&file, 5, 4) ||                         /* PF_R | PF_X */
        !number(&file, 0, 8) ||                         /* file offset */
        !number(&file, ELF_BASE_ADDRESS, 8) ||          /* virtual address */
        !number(&file, ELF_BASE_ADDRESS, 8) ||          /* physical address */
        !number(&file, file_size, 8) ||                 /* bytes in file */
        !number(&file, file_size, 8) ||                 /* bytes in memory */
        !number(&file, 0x1000, 8))                      /* alignment */
    {
        goto done;
    }

    /* Pad from the end of the headers to file offset 0x1000. */
    while (file.count < ELF_PAYLOAD_OFFSET)
    {
        if (!byte_push(&file, 0))
            goto done;
    }

    if (!append(&file, code->data, code->count))
        goto done;

    okay = write_binary(&file, path);

done:
    free(file.data);
    return okay;
}
