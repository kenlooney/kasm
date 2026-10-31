#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <iostream>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    for (const char* spelling : {"mov", "move", "put"}) {
        if (!parser.is_move_alias(spelling)) {
            std::cerr << "Expected move alias: " << spelling << '\n';
            return 1;
        }
    }
    for (const char* spelling : {"into", "to", "unknown", ""}) {
        if (parser.is_move_alias(spelling)) {
            std::cerr << "Unexpected move alias: " << spelling << '\n';
            return 1;
        }
    }
    return 0;
}
