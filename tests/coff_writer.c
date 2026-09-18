#include "coff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Read serialized fields independently of the writer. */
static unsigned field(const unsigned char *p, unsigned width)
{
    unsigned value = 0;
    for (unsigned i = 0; i < width; ++i) value |= (unsigned)p[i] << (8 * i);
    return value;
}
#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "COFF check failed at line %d\n", __LINE__); return 1; } } while (0)

int main(int argc, char **argv)
{
    REQUIRE(argc == 2);
    Bytes text = {0};
    REQUIRE(byte_push(&text, 0xe9));
    for (int i = 0; i < 4; ++i) REQUIRE(byte_push(&text, 0));
    CoffSymbol symbols[] = {{"answer", 0, 1, 1}, {"helper", 0, 0, 1}};
    CoffRelocation relocation = {1, 1, COFF_REL32};
    CoffObject object = {&text, symbols, 2, &relocation, 1};
    REQUIRE(write_coff(&object, argv[1]));
    FILE *f = fopen(argv[1], "rb");
    REQUIRE(f != NULL);
    unsigned char data[115];
    REQUIRE(fread(data, 1, sizeof data, f) == sizeof data);
    REQUIRE(fgetc(f) == EOF);
    REQUIRE(fclose(f) == 0);
    REQUIRE(field(data, 2) == 0x8664 && field(data + 2, 2) == 1);
    REQUIRE(field(data + 8, 4) == 75 && field(data + 12, 4) == 2);
    REQUIRE(memcmp(data + 20, ".text", 5) == 0);
    REQUIRE(field(data + 36, 4) == 5 && field(data + 40, 4) == 60);
    REQUIRE(field(data + 44, 4) == 65 && field(data + 52, 2) == 1);
    REQUIRE(field(data + 56, 4) == 0x60500020);
    REQUIRE(data[60] == 0xe9 && field(data + 61, 4) == 0);
    REQUIRE(field(data + 65, 4) == 1 && field(data + 69, 4) == 1);
    REQUIRE(field(data + 73, 2) == COFF_REL32);
    REQUIRE(memcmp(data + 75, "answer", 6) == 0 && field(data + 87, 2) == 1);
    REQUIRE(memcmp(data + 93, "helper", 6) == 0 && field(data + 105, 2) == 0);
    REQUIRE(data[91] == 2 && data[109] == 2 && field(data + 111, 4) == 4);

    /* Leave the REL32 fixture above for the native linker test. */
    char *other = malloc(strlen(argv[1]) + 8);
    REQUIRE(other != NULL);
    sprintf(other, "%s.extra", argv[1]);
    relocation.offset = 4;
    REQUIRE(!write_coff(&object, other));
    relocation.offset = 1;
    relocation.symbol = 2;
    REQUIRE(!write_coff(&object, other));
    relocation.symbol = 1;
    text.patch_count = 1;
    REQUIRE(!write_coff(&object, other));
    text.patch_count = 0;
    symbols[1].name = "answer";
    REQUIRE(!write_coff(&object, other));
    symbols[1].name = "a_symbol_name_longer_than_eight_bytes";
    for (int i = 0; i < 8; ++i) REQUIRE(byte_push(&text, 0));
    relocation.offset = 5;
    relocation.kind = COFF_ADDR64;
    REQUIRE(write_coff(&object, other));
    f = fopen(other, "rb");
    REQUIRE(f != NULL);
    unsigned char extended[256];
    size_t count = fread(extended, 1, sizeof extended, f);
    REQUIRE(fclose(f) == 0);
    REQUIRE(field(extended + 81, 2) == COFF_ADDR64);
    REQUIRE(field(extended + 101, 4) == 0 && field(extended + 105, 4) == 4);
    REQUIRE(count == 123 + strlen(symbols[1].name) + 1);
    REQUIRE(strcmp((const char *)extended + 123, symbols[1].name) == 0);
    object.relocation_count = 0;
    for (int i = 0; i < 0x4000; ++i) REQUIRE(byte_push(&text, 0x90));
    REQUIRE(write_coff(&object, other));
    f = fopen(other, "rb");
    REQUIRE(f != NULL && fseek(f, 0, SEEK_END) == 0);
    REQUIRE(ftell(f) > 0x3000);
    REQUIRE(fclose(f) == 0);
    remove(other);
    free(other);
    free(text.data);
    return 0;
}
