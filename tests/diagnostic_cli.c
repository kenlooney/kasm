#include "kasm/diagnostic.h"
#include "kasm/source.h"
#include <stdio.h>

int main(int argc, char **argv) {
    Source source;
    if (argc != 2) {
        fprintf(stderr, "usage: lesson input.asm\n");
        return 1;
    }
    if (source_load(&source, argv[1]) != 0)
        return 1;

    Span span = {0, source.length};
    diagnostic(&source, span, "demonstration diagnostic");
    return 1;
}
