
#include "source.hpp"
#include "diagnostic.hpp"
#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <iostream>

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
    for (const char* text : {"2 +", "2 3", "18446744073709551615 + 1"}) {
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
