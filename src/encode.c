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
        case ST_NOP:
            if (!byte_push(bytes, 0x90))
            {
                return 0;
            }
            break;
        case ST_CLC:
            if (!byte_push(bytes, 0xF8))
            {
                return 0;
            }
            break;
        case ST_STC:
            if (!byte_push(bytes, 0xF9))
            {
                return 0;
            }
            break;
        case ST_CMC:
            if (!byte_push(bytes, 0xF5))
            {
                return 0;
            }
            break;
        case ST_CLD:
            if (!byte_push(bytes, 0xFC))
            {
                return 0;
            }
            break;
        case ST_STD:
            if (!byte_push(bytes, 0xFD))
            {
                return 0;
            }
            break;

        case ST_LAHF:
            if (!byte_push(bytes, 0x9F))
            {
                return 0;
            }
            break;
        case ST_SAHF:
            if (!byte_push(bytes, 0x9E))
            {
                return 0;
            }
            break;
        case ST_PUSHF:
            if (!byte_push(bytes, 0x9C))
            {
                return 0;
            }
            break;
        case ST_POPF:
            if (!byte_push(bytes, 0x9D))
            {
                return 0;
            }
            break;
        case ST_CBW:
            if (!byte_push(bytes, 0x98))
            {
                return 0;
            }
            break;
        case ST_CWD:
            if (!byte_push(bytes, 0x99))
            {
                return 0;
            }
            break;
        case ST_IRET:
            if (!byte_push(bytes, 0xCF))
            {
                return 0;
            }
            break;
        // Single-byte opcode with encoded register
        case ST_INC:
            if (statement->reg16 > REG16_DI)
                return 0;
            if (!byte_push(bytes, (uint8_t)(0x40u + (unsigned)statement->reg16)))
                return 0;
            break;

        case ST_DEC:
            if (statement->reg16 > REG16_DI)
                return 0;

            if (!byte_push(
                    bytes,
                    (uint8_t)(0x48u + (unsigned)statement->reg16)))
                return 0;
            break;
        case ST_PUSH:
            if (statement->reg16 > REG16_DI)
                return 0;

            if (!byte_push(
                    bytes,
                    (uint8_t)(0x50u + (unsigned)statement->reg16)))
                return 0;
            break;
        case ST_POP:
            if (statement->reg16 > REG16_DI)
                return 0;

            if (!byte_push(
                    bytes,
                    (uint8_t)(0x58u + (unsigned)statement->reg16)))
                return 0;
            break;
        case ST_XCHG:
            if (statement->reg16 > REG16_DI)
                return 0;

            if (!byte_push(
                    bytes,
                    (uint8_t)(0x90u + (unsigned)statement->reg16)))
                return 0;
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

        // ORG
        case ST_ORG:
            // ORG does not emit any bytes
            break;
        // JMPFAR
        case ST_JMPFAR:
            if (!byte_push(bytes, 0xEAu) ||
                !little_endian(bytes, (uint64_t)statement->far_offset, 2) ||
                !little_endian(bytes, (uint64_t)statement->value, 2))
                return 0;
            break;

        // Data definition instructions (db, dw, dd)
        case ST_DB:
            if (!byte_push(bytes, (uint8_t)statement->value))
            {
                return 0;
            }
            break;

        case ST_DW:
            if (!little_endian(
                    bytes,
                    (uint64_t)statement->value,
                    2))
            {
                return 0;
            }
            break;

        case ST_DD:
            if (!little_endian(
                    bytes,
                    (uint64_t)statement->value,
                    4))
            {
                return 0;
            }
            break;

        case ST_PADTO:
            /*
             * Layout and emission must agree about where PADTO begins.
             * Semantic analysis and layout have already established that
             * the destination is not behind this offset.
             */
            if (bytes->count != statement->offset)
                return 0;

            while (bytes->count < (size_t)statement->value)
            {
                if (!byte_push(bytes, (uint8_t)statement->fill_value))
                    return 0;
            }
            break;

        case ST_MOV:
            if (statement->reg16 > REG16_DI)
                return 0;

            if (!byte_push(
                    bytes,
                    (uint8_t)(0xB8u + (unsigned)statement->reg16)) ||
                !little_endian(bytes, (uint64_t)statement->value, 2))
            {
                return 0;
            }
            break;

        case ST_MOV_SEGMENT:
        {
            uint8_t modrm;

            if (statement->segment_register != SEG_ES &&
                statement->segment_register != SEG_SS &&
                statement->segment_register != SEG_DS)
                return 0;

            if (statement->reg16 != REG16_AX)
                return 0;

            modrm = (uint8_t)(0xC0u |
                              ((unsigned)statement->segment_register << 3) |
                              (unsigned)statement->reg16);

            if (!byte_push(bytes, 0x8Eu) ||
                !byte_push(bytes, modrm))
                return 0;
            break;
        }

        default:
            return 0;
        }
    }

    return 1;
}
