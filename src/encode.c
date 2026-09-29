#include "kasm/encode.h"

int emit_program(
    const Program *program,
    const Target *target,
    Bytes *bytes)
{
    if (!program || !target || !bytes)
        return 0;

    if (target->arch != ARCH_X86 || target->mode != MODE_16)
        return 0;

    for (int i = 0; i < program->count; i++)
    {
        const Statement *statement = &program->statements[i];

        switch (statement->kind)
        {

        // No-operand instructions
        case ST_HLT:
            if (!byte_push(bytes, 0xF4))
            {
                return 0;
            }
            break;
        case ST_CLI:
            if (!byte_push(bytes, 0xFA))
            {
                return 0;
            }
            break;
        case ST_STI:
            if (!byte_push(bytes, 0xFB))
            {
                return 0;
            }
            break;
        case ST_LABEL:
            // Labels do not emit any bytes
            break;

        // Jump instructions
        case ST_JMP8:
            if (!byte_push(bytes, 0xEB) ||
                !byte_push(bytes, (uint8_t)statement->value))
            {
                return 0;
            }
            break;

        case ST_MOV:
            if (!byte_push(bytes, 0xB8) ||
                !little_endian(
                    bytes,
                    (uint64_t)statement->value,
                    2))
            {
                return 0;
            }
            break;

        default:
            return 0;
        }
    }

    return 1;
}