#include <stdio.h>
#ifndef KASM_GENERATED_HEADER
#define KASM_GENERATED_HEADER "program.h"
#endif
#include KASM_GENERATED_HEADER

int main(void) {
    printf("Size: %zu\n", code_size);

    for (size_t i = 0; i < code_size; i++)
        printf("%02X ", (unsigned int)code[i]);

    puts("");
  
}
