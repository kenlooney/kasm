#include "source.hpp"
#include "diagnostic.hpp"
#include "kasmlexer.hpp"

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
    KasmLexer lexer;
    for (const Token& token : lexer.tokenize(source.text)) {
        std::cout << token.span.begin << ".." << token.span.end << " "
                  << token.kind << " " << token.text << "\n";
    }
    return 0;
}
