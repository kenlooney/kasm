#if !defined(_WIN32)
#define _DEFAULT_SOURCE
#endif
#ifndef KASM_GENERATED_HEADER
#define KASM_GENERATED_HEADER "program.h"
#endif
#include KASM_GENERATED_HEADER
#include <stdio.h>
#include <string.h>
#include "relocate.h"
#if defined(_WIN32)
#include <windows.h>
#else
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
    void *memory = VirtualAlloc(NULL, code_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (memory == NULL) {
        fprintf(stderr, "VirtualAlloc failed\n");
        return 1;
    }
    memcpy(memory, code, code_size);
    if (!relocate(memory, code_size, patch_offsets, patch_count)) {
        VirtualFree(memory, 0, MEM_RELEASE);
        return 1;
    }
    DWORD old_protection;
    if (!VirtualProtect(memory, code_size, PAGE_EXECUTE_READ, &old_protection) ||
        !FlushInstructionCache(GetCurrentProcess(), memory, code_size)) {
        fprintf(stderr, "executable memory setup failed\n");
        VirtualFree(memory, 0, MEM_RELEASE);
        return 1;
    }
#else

    void *memory =
        mmap(NULL, code_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    memcpy(memory, code, code_size);
    if (!relocate(memory, code_size, patch_offsets, patch_count)) {
        munmap(memory, code_size);
        return 1;
    }
    if (mprotect(memory, code_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        munmap(memory, code_size);
        return 1;
    }
#endif

    int (*function)(void);
    _Static_assert(sizeof function == sizeof memory, "x86-64 pointer sizes must match");
    /* Platform-specific executable-memory bridge, not portable ISO C. */
    memcpy(&function, &memory, sizeof function);
    int result = function();
    printf("result = %d\n", result);
#if defined(_WIN32)
    if (!VirtualFree(memory, 0, MEM_RELEASE))
        return 1;
#else
    if (munmap(memory, code_size) != 0)
        return 1;
#endif
    return 0;
#endif
}
