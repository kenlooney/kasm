#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    std::uint64_t value = 0;
    std::string error;

    for (const auto& example : {std::pair<const char*, std::uint64_t>{"(2 + 3) * 4", 20},
                                {"2 * (3 + (4 * 5))", 46},
                                {"((7))", 7}}) {
        if (!parser.parse_expression(lexer.tokenize(example.first), value, error) || value != example.second) {
            std::cerr << "Unexpected result for expression: " << example.first << "\n";
            return 1;
        }
    }

    for (const char* expression : {"(2 + 3", "2 + 3)", "()", "(2 + )", "(2 3)"}) {
        error.clear();
        if (parser.parse_expression(lexer.tokenize(expression), value, error) || error.empty()) {
            std::cerr << "Expected an error for expression: " << expression << "\n";
            return 1;
        }
    }
    return 0;
}
