#include "kasm/encode.h"

int emit_program(
    const Program *program,
    const Target *target,
    Bytes *bytes
) {
    if (!program || !target || !bytes)
        return 0;

    if (target->arch != ARCH_X86 || target->mode != MODE_16)
        return 0;

    for (int i = 0; i < program->count; i++) {
        const Statement *statement = &program->statements[i];

        switch (statement->kind) {
        case ST_MOV:
            if (!byte_push(bytes, 0xB8) ||
                !little_endian(
                    bytes,
                    (uint64_t)statement->value,
                    2
                )) {
                return 0;
            }
            break;

        default:
            return 0;
        }
    }

    return 1;
}