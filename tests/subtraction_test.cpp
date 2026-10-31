#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    parser.add_operator_alias({"+", {"add"}});
    parser.add_operator_alias({"-", {"take", "minus"}});
    std::uint64_t value = 0;
    std::string error;
    for (const auto& example : {std::pair<const char*, std::uint64_t>{"10 - 3", 7},
                                {"10 take 3", 7},
                                {"10 minus 3", 7},
                                {"10 - 3 add 2", 9},
                                {"20 minus 3 * 4", 8},
                                {"(10 take 3) * 2", 14},
                                {"9 - 3 - 2", 4},
                                {"3 - 3", 0}}) {
        error.clear();
        if (!parser.parse_expression(lexer.tokenize(example.first), value, error) || value != example.second) {
            std::cerr << "Unexpected result for " << example.first << ": " << value << " " << error << "\n";
            return 1;
        }
    }
    for (const char* expression : {"3 - 5", "3 take 5", "3 minus 5", "3 -"}) {
        error.clear();
        if (parser.parse_expression(lexer.tokenize(expression), value, error) || error.empty()) {
            std::cerr << "Expected an error for " << expression << "\n";
            return 1;
        }
    }
    return 0;
}
