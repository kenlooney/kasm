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
    for (const auto& example : {std::pair<const char*, std::uint64_t>{"2 + 3", 5},
                                {"2 add 3", 5},
                                {"2 plus 3", 5},
                                {"8 - 3", 5},
                                {"8 sub 3", 5},
                                {"8 minus 3", 5},
                                {"2 * 3", 6},
                                {"2 mul 3", 6},
                                {"2 add 3 mul 4", 14},
                                {"2 times 3", 6},
                                {"(2 add 3) * 4", 20}}) {
        if (!parser.parse_expression(lexer.tokenize(example.first), value, error) || value != example.second) {
            std::cerr << "Unexpected result for expression: " << example.first << "\n";
            return 1;
        }
    }
    if (parser.parse_expression(lexer.tokenize("2 unknown 3"), value, error)) {
        std::cerr << "Expected an error for an unknown operator\n";
        return 1;
    }
    return 0;
}
