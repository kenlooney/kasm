#include "kasm/kasm.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
    Source source;
    if (source_load(&source, argv[1]) != 0) {
        fprintf(stderr, "Failed to load source file: %s\n", argv[1]);
        return 1;
    }
    printf("kasm %s\n", kasm_version());
    return 0;
}
