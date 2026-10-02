#include "kasm/kasm.h"
#include "kasm/layout.h"
#include "kasm/program.h"
#include "kasm/target.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGIFY_VALUE(value) #value
#define STRINGIFY(value) STRINGIFY_VALUE(value)
#define EXPECTED_VERSION_STRING \
    STRINGIFY(KASM_VERSION_MAJOR) "." \
    STRINGIFY(KASM_VERSION_MINOR) "." \
    STRINGIFY(KASM_VERSION_PATCH)

static int test_version(void)
{
    if (strcmp(KASM_VERSION_STRING, EXPECTED_VERSION_STRING) != 0) {
        fprintf(stderr,
                "version components do not match version string: %d.%d.%d != %s\n",
                KASM_VERSION_MAJOR,
                KASM_VERSION_MINOR,
                KASM_VERSION_PATCH,
                KASM_VERSION_STRING);
        return 1;
    }

    if (strcmp(kasm_version(), KASM_VERSION_STRING) != 0) {
        fprintf(stderr, "unexpected runtime version: %s\n", kasm_version());
        return 1;
    }

    return 0;
}

static int test_semantic_rejection(const char *text, const char *description)
{
    const Target target = {
        .arch = ARCH_X86,
        .mode = MODE_16
    };
    Source source = {
        .path = description,
        .text = text,
        .length = strlen(text)
    };
    Parser parser = {0};
    Program program = {0};
    int result = 1;

    parser_start(&parser, &source);

    if (!parse_program(&parser, &program, &target))
    {
        fprintf(stderr, "%s: parser rejected input before semantic validation\n",
                description);
        goto cleanup;
    }

    if (!evaluate_program(&parser, &program))
    {
        fprintf(stderr,
                "%s: expression evaluation failed before semantic validation\n",
                description);
        goto cleanup;
    }

    if (!layout_program(&program, &target))
    {
        fprintf(stderr, "%s: layout failed before semantic validation\n",
                description);
        goto cleanup;
    }

    if (check_program(&parser, &program, &target))
    {
        fprintf(stderr, "%s: expected semantic validation to reject input\n",
                description);
        goto cleanup;
    }

    result = 0;

cleanup:
    free(program.statements);
    free(parser.nodes);
    return result;
}

static int test_parser_rejection(const char *text, const char *description)
{
    const Target target = {
        .arch = ARCH_X86,
        .mode = MODE_16
    };
    Source source = {
        .path = description,
        .text = text,
        .length = strlen(text)
    };
    Parser parser = {0};
    Program program = {0};
    int result = 1;

    parser_start(&parser, &source);

    if (parse_program(&parser, &program, &target))
    {
        fprintf(stderr, "%s: expected parser to reject input\n", description);
        goto cleanup;
    }

    result = 0;

cleanup:
    free(program.statements);
    free(parser.nodes);
    return result;
}

static int test_encoded_register_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "dec eax;",
        "dec rejects a register with the wrong width");
    failures += test_semantic_rejection(
        "push banana;",
        "push rejects an unknown register");
    failures += test_parser_rejection(
        "pop;",
        "pop requires a register operand");
    failures += test_semantic_rejection(
        "xchg cx, dx;",
        "compact xchg requires ax as its first operand");

    return failures != 0;
}

static int test_segment_mov_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "mov cs, ax;",
        "segment mov rejects cs as a destination");
    failures += test_semantic_rejection(
        "mov ds, cx;",
        "segment mov requires ax as its source");
    failures += test_semantic_rejection(
        "mov banana, ax;",
        "segment mov rejects an unknown destination register");

    return failures != 0;
}

static int test_far_jump_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "jmpfar 0, missing;",
        "far jump rejects an undefined label");
    failures += test_semantic_rejection(
        "jmpfar 65536, target; target:",
        "far jump rejects a segment outside 16 bits");
    failures += test_semantic_rejection(
        "org 65535; jmpfar 0, target; target:",
        "far jump rejects a destination outside 16 bits");
    failures += test_parser_rejection(
        "jmpfar 0 target;",
        "far jump requires a comma before its target");

    return failures != 0;
}

static int test_mode_rejections(void)
{
    int failures = 0;

    failures += test_parser_rejection(
        "mode;",
        "mode requires a mode number");
    failures += test_parser_rejection(
        "mode 24;",
        "mode rejects an unsupported mode number");
    failures += test_parser_rejection(
        "mode 32",
        "mode requires a semicolon");

    return failures != 0;
}

static int test_mode_width_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "mode 16; mov eax, 1;",
        "mode 16 rejects a 32-bit destination register");
    failures += test_semantic_rejection(
        "mode 32; mov ax, 1;",
        "mode 32 rejects a 16-bit destination register");
    failures += test_semantic_rejection(
        "mode 64; mov rax, 1;",
        "mode 64 rejects unimplemented 64-bit mov");

    return failures != 0;
}
static int test_symbol_expression_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "dw missing;",
        "data rejects an undefined symbol");
    failures += test_semantic_rejection(
        "org 0x7C00; db later; later: db 0;",
        "resolved data still has to fit its width");

    return failures != 0;
}

static int test_equ_parser_rejections(void)
{
    int failures = 0;

    failures += test_parser_rejection(
        "answer equ;",
        "equ requires an expression");
    failures += test_parser_rejection(
        "answer equ 42",
        "equ requires a semicolon");

    return failures != 0;
}
static int test_equ_semantic_rejections(void)
{
    int failures = 0;

    failures += test_semantic_rejection(
        "name: name equ 1;",
        "a label and equ cannot share a name");
    failures += test_semantic_rejection(
        "value equ missing; dw value;",
        "equ rejects an undefined label");
    failures += test_semantic_rejection(
        "first equ 1; second equ first + 1; dw second;",
        "the first equ implementation rejects equ chains");

    return failures != 0;
}

int main(void)
{
    int failures = 0;

    failures += test_version();
    failures += test_encoded_register_rejections();
    failures += test_segment_mov_rejections();
    failures += test_far_jump_rejections();
    failures += test_mode_rejections();
    failures += test_mode_width_rejections();
    failures += test_equ_parser_rejections();
    failures += test_equ_semantic_rejections();
    failures += test_symbol_expression_rejections();
    

    return failures != 0;
}
