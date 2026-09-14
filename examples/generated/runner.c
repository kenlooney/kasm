#if !defined(_WIN32)
#define _DEFAULT_SOURCE
#endif
#include "generated.h"
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
#include <sys/mman.h>
#endif

int main(void) {
#if !defined(__x86_64__) && !defined(_M_X64)
    fprintf(stderr, "execution requires an x86-64 process\n");
    return 1;
#else
    if (code_size == 0) {
        fprintf(stderr, "empty program\n");
        return 1;
    }

#if defined(_WIN32)
    puts("Linux execution harness; Windows arrives next chapter");
    return 0;
#else

    void *memory =
        mmap(NULL, code_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    memcpy(memory, code, code_size);

    if (mprotect(memory, code_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        munmap(memory, code_size);
        return 1;
    }

    int (*function)(void);
    _Static_assert(sizeof function == sizeof memory, "x86-64 pointer sizes must match");
    /* Platform-specific executable-memory bridge, not portable ISO C. */
    memcpy(&function, &memory, sizeof function);
    int result = function();
    printf("result = %d\n", result);
    if (munmap(memory, code_size) != 0)
        return 1;
    return 0;
#endif
#endif
}
