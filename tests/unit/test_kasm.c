#include "kasm/kasm.h"

#include <stdio.h>
#include <string.h>

static int test_greeting(void)
{
    const char *greeting = kasm_greeting();

    if (greeting == NULL) {
        fputs("kasm_greeting returned NULL\n", stderr);
        return 1;
    }

    if (strcmp(greeting, "Hello, Kasm!") != 0) {
        fprintf(stderr, "unexpected greeting: %s\n", greeting);
        return 1;
    }

    return 0;
}

int main(void)
{
    return test_greeting();
}
