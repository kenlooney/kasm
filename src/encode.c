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

#include "source.h"
#include "program.h"
#include "emit.h"
#include <stdint.h>
#include "layout.h"
static int little_endian(Bytes *bytes, uint64_t value, int width)
{
    for (int i = 0; i < width; i++)
    {
        if (!byte_push(bytes, (unsigned char)(value & 255)))
            return 0;
        value >>= 8;
    }
    return 1;
}
int encode(const Source *source, Program *program, Bytes *bytes, const Target *target)
{
    (void)source;
    for (int i = 0; i < program->count; i++)
    {
        Statement *s = &program->statements[i];
        size_t size = instruction_size(s);
        
        
        // mov
        if (s->kind == ST_MOV)
        {

            switch (s->reg_code)
            {
            case 0: // eax
                switch (target->mode)
                {
                case MODE_16:
                    if (!byte_push(bytes, 0xB8) || !little_endian(bytes, (uint64_t)s->value, 2))
                        return 0;
                    break;

                case MODE_32:
                case MODE_64:
                default:
                    if (!byte_push(bytes, 0xB8) || !little_endian(bytes, (uint64_t)s->value, 4))
                        return 0;
                    break;
                }
                break;

            case 1: // ecx
                switch (target->mode)
                {
                case MODE_16:
                    if (!byte_push(bytes, 0xB9) || !little_endian(bytes, (uint64_t)s->value, 2))
                        return 0;
                    break;

                case MODE_32:
                case MODE_64:
                default:
                    if (!byte_push(bytes, 0xB9) || !little_endian(bytes, (uint64_t)s->value, 4))
                        return 0;
                    break;
                }
                break;

            case 2: // edx
                switch (target->mode)
                {
                case MODE_16:
                    if (!byte_push(bytes, 0xBA) || !little_endian(bytes, (uint64_t)s->value, 2))
                        return 0;
                    break;

                case MODE_32:
                case MODE_64:
                default:
                    if (!byte_push(bytes, 0xBA) || !little_endian(bytes, (uint64_t)s->value, 4))
                        return 0;
                    break;
                }
                break;
            default:
                return 0;
            }
        }

        // Data Directives (e.g., DB)
        else if (s->kind == ST_DB) {
            for (size_t j = 0; j < s->data_count; j++) {
                const DataElement *element = &program->data[s->data_start + j];
                if(!little_endian(bytes, (uint64_t)element->value, (int)s->data_width)) 
                    return 0;
            }
        }
        // Data Directives (e.g., DW)
        else if (s->kind == ST_DW) {
            for (size_t j = 0; j < s->data_count; j++) {
                const DataElement *element = &program->data[s->data_start + j];
                if(!little_endian(bytes, (uint64_t)element->value, (int)s->data_width)) 
                    return 0;
            }
        }

        // ret
        else if (s->kind == ST_RET)
        {
            if (!byte_push(bytes, 0xC3))
                return 0;
        }
        // dec eax
        else if (s->kind == ST_DEC)
        {
            if (!byte_push(bytes, 0xFF) || !byte_push(bytes, 0xC8))
                return 0;
        }
        // or rim
        else if (s->kind == ST_OR)
        {
            if (!byte_push(bytes, 0x0D) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
        // xor rim
        else if (s->kind == ST_XOR)
        {
            if (!byte_push(bytes, 0x35) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
        // and rim
        else if (s->kind == ST_AND)
        {
            if (!byte_push(bytes, 0x25) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
        // cmp rim
        else if (s->kind == ST_CMP_RIM)
        {
            if (!byte_push(bytes, 0x3D) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
        // adc rim
        else if (s->kind == ST_ADC)
        {
            if (!byte_push(bytes, 0x15) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
        // int imm8
        else if (s->kind == ST_INT_IMM8)
        {
            if (!byte_push(bytes, 0xCD) || !little_endian(bytes, (uint64_t)s->value, 1))
                return 0;
        }

        // inc eax
        else if (s->kind == ST_INC)
        {
            if (!byte_push(bytes, 0xFF) || !byte_push(bytes, 0xC0))
                return 0;
        }
        // push eax
        else if (s->kind == ST_PUSH)
        {
            if (!byte_push(bytes, 0x50))
                return 0;
        }
        // pop eax
        else if (s->kind == ST_POP)
        {
            if (!byte_push(bytes, 0x58))
                return 0;
        }
        // ADD EAX, imm32: EAX is implicit in opcode 05.
        else if (s->kind == ST_ADD_RIM)
        {
            switch (s->reg_code)
            {
            case 0: // eax
                if (!byte_push(bytes, 0x05) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            case 1: // ecx
                if (!byte_push(bytes, 0x81) || !byte_push(bytes, 0xC1) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            case 2: // edx
                if (!byte_push(bytes, 0x81) || !byte_push(bytes, 0xC2) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            default:
                return 0;
            }
        }
        // SUB EAX, imm32: EAX is implicit in opcode 05.
        else if (s->kind == ST_SUB_RIM)
        {
            switch (s->reg_code)
            {
            case 0: // eax
                if (!byte_push(bytes, 0x2D) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            case 1: // ecx
                if (!byte_push(bytes, 0x81) || !byte_push(bytes, 0xE9) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            case 2: // edx
                if (!byte_push(bytes, 0x81) || !byte_push(bytes, 0xEA) || !little_endian(bytes, (uint64_t)s->value, 4))
                    return 0;
                break;
            default:
                return 0;
            }
        }
        // Conditional near jumps share the same relative displacement format.
        else if (s->kind == ST_JNZ || s->kind == ST_JZ || s->kind == ST_JB || s->kind == ST_JL)
        {
            size_t target;
            if (!label_offset(source, program, s->operand, &target))
                return 0;
            if (size != 6)
            {
                diagnostic(source, s->operand.span, "conditional jump must be 6 bytes");
                return 0;
            }
            long long displacement = (long long)target - (long long)(s->offset + size);
            if (displacement < INT32_MIN || displacement > INT32_MAX)
            {
                diagnostic(source, s->operand.span, "conditional jump outside signed 32-bit range");
                return 0;
            }
            unsigned char opcode = s->kind == ST_JNZ ? 0x85 : s->kind == ST_JZ ? 0x84
                                                          : s->kind == ST_JB   ? 0x82
                                                                               : 0x8C;
            if (!byte_push(bytes, 0x0F) || !byte_push(bytes, opcode) ||
                !little_endian(bytes, (uint64_t)displacement, 4))
                return 0;
        }
        // short jmp
        else if (s->kind == ST_SHORT_JMP)
        {
            size_t target;
            if (!label_offset(source, program, s->operand, &target))
                return 0;
            if (size != 2)
            {
                diagnostic(source, s->operand.span, "short jump must be 2 bytes");
                return 0;
            }
            long long displacement = (long long)target - (long long)(s->offset + instruction_size(s));
            if (displacement < INT8_MIN || displacement > INT8_MAX)
            {
                diagnostic(source, s->operand.span, "short jump outside signed 8-bit range");
                return 0;
            }
            if (!byte_push(bytes, 0xEB) || !little_endian(bytes, (uint64_t)displacement, 1))
                return 0;
        }
        // near jmp
        else if (s->kind == ST_NEAR_JMP)
        {
            size_t target;
            if (!label_offset(source, program, s->operand, &target))
                return 0;
            if (size != 5)
            {
                diagnostic(source, s->operand.span, "near jump must be 5 bytes");
                return 0;
            }
            long long displacement = (long long)target - (long long)(s->offset + size);
            if (displacement < INT32_MIN || displacement > INT32_MAX)
            {
                diagnostic(source, s->operand.span, "near jump outside signed 32-bit range");
                return 0;
            }
            if (!byte_push(bytes, 0xE9) || !little_endian(bytes, (uint64_t)displacement, 4))
                return 0;
        }
        // abs jmp
        else if (s->kind == ST_ABS_JMP)
        {
            size_t target;
            if (!label_offset(source, program, s->operand, &target))
                return 0;
            if (size != 14)
            {
                diagnostic(source, s->operand.span, "abs jump must be 14 bytes");
                return 0;
            }
            if (bytes->patch_count >= sizeof bytes->patches / sizeof bytes->patches[0])
            {
                diagnostic(source, s->operand.span, "too many relocation patches");
                return 0;
            }
            // RIP points to the address slot immediately after these six bytes.
            if (!byte_push(bytes, 0xFF) || !byte_push(bytes, 0x25) ||
                !little_endian(bytes, 0, 4))
                return 0;
            size_t patch_offset = bytes->count;
            // The loader replaces this image offset with the absolute address.
            if (!little_endian(bytes, (uint64_t)target, 8))
                return 0;
            bytes->patches[bytes->patch_count++] = patch_offset;
        }
    }
    return 1;
}
