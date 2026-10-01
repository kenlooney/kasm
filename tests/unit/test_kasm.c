#include "kasm/kasm.h"
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

    if (!parse_program(&parser, &program))
    {
        fprintf(stderr, "%s: parser rejected input before semantic validation\n",
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
    Source source = {
        .path = description,
        .text = text,
        .length = strlen(text)
    };
    Parser parser = {0};
    Program program = {0};
    int result = 1;

    parser_start(&parser, &source);

    if (parse_program(&parser, &program))
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

int main(void)
{
    int failures = 0;

    failures += test_version();
    failures += test_encoded_register_rejections();
    failures += test_segment_mov_rejections();

    return failures != 0;
}
