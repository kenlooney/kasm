#include "framework/source.hpp"
#include "framework/diagnostic.hpp"
#include "kasm/kasmlexer.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    KasmLexer lexer;
    const auto tokens = lexer.tokenize("mov 0x2a, // comment\n/* comment */42");
    const Token expected[] = {
        {"identifier", "mov", {0, 3}},
        {"integer", "0x2a", {4, 8}},
        {"punctuation", ",", {8, 9}},
        {"integer", "42", {34, 36}}
    };
    if (tokens.size() != 4) {
        std::cerr << "Unexpected token count\n";
        return 1;
    }
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].kind != expected[i].kind ||
            tokens[i].text != expected[i].text ||
            tokens[i].span.begin != expected[i].span.begin ||
            tokens[i].span.end != expected[i].span.end) {
            std::cerr << "Unexpected token at index " << i << "\n";
            return 1;
        }
    }
    if (argc != 2) {
        report("<command line>", 1, 1, "Expected a source file");
        return 1;
    }
    Source source;
    if (!Source::load(argv[1], source)) {
        report(argv[1], 1, 1, "Could not open file");
        return 1;
    }
    for (const Token& token : lexer.tokenize(source.text)) {
        std::cout << token.span.begin << ".." << token.span.end << " "
                  << token.kind << " " << token.text << "\n";
    }
    return 0;
}
