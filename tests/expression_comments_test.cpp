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

    for (const auto& example : {
             std::pair<const char*, std::uint64_t>{"// first expression\n2 + 3 * 4 // end of line", 14},
             {"(2 + /* inline comment */ 3) * 4", 20},
             {"/* multi-line\ncomment */ 10 - 3", 7}}) {
        error.clear();
        if (!parser.parse_expression(lexer.tokenize(example.first), value, error) || value != example.second) {
            std::cerr << "Failed to parse expression with comments: " << error << "\n";
            return 1;
        }
    }

    return 0;
}
