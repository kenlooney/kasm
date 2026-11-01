
#include "framework/source.hpp"
#include "framework/diagnostic.hpp"
#include "kasm/kasmlexer.hpp"
#include "kasm/kasmparser.hpp"
#include <iostream>
#include <utility>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        report("<command line>", 1, 1, "Expected a source file");
        return 1;
    }
    Source source;
    if (!Source::load(argv[1], source)) {
        report(argv[1], 1, 1, "Could not open file");
        return 1;
    }
    std::uint64_t value = 0;
    std::string error;
    KasmLexer lexer;
    KasmParser parser(lexer);
    if (!parser.parse_expression(lexer.tokenize("2 + 3 + 0b100"), value, error) || value != 9) {
        std::cerr << "Expected 2 + 3 + 0b100 to evaluate to 9\n";
        return 1;
    }
    for (const auto& example : {std::pair<const char*, std::uint64_t>{"2 + 3 * 4", 14},
                                {"2 * 3 + 4 * 5", 26},
                                {"2 * 3 * 4", 24},
                                {"18446744073709551615 * 0", 0}}) {
        if (!parser.parse_expression(lexer.tokenize(example.first), value, error) || value != example.second) {
            std::cerr << "Unexpected result for expression: " << example.first << "\n";
            return 1;
        }
    }
    for (const char* text : {"2 +", "2 *", "2 3", "18446744073709551615 + 1",
                             "18446744073709551615 * 2"}) {
        error.clear();
        if (parser.parse_expression(lexer.tokenize(text), value, error) || error.empty()) {
            std::cerr << "Expected an error for expression: " << text << "\n";
            return 1;
        }
    }
    value = 0;
    error.clear();
    if (!parser.parse_expression(lexer.tokenize(source.text), value, error)) {
        std::cerr << source.name << ": " << error << "\n";
        return 1;
    }
    std::cout << value << "\n";

    
    return 0;
}
