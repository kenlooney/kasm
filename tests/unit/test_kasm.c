#include "kasm/kasm.h"

#include <stdio.h>
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

int main(void)
{
    return test_version();
}
