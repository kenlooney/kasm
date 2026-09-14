#include <stdio.h>
#include "generated.h"

int main(void) {
    printf("Size: %zu\n", code_size);

    for (size_t i = 0; i < code_size; i++)
        printf("%02X ", (unsigned int)code[i]);

    puts("");
  
}