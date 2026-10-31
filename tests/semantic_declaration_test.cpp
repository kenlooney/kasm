#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "kasmsemantic.hpp"
#include <iostream>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    KasmSemanticAnalyzer analyzer(parser);
    std::string error;
    for (const char* source : {"declare 0 as byte", "declare 255 as byte", "set 200 plus 55 being byte",
                               "declare 1 as byte\ndeclare 255 as byte"}) {
        error.clear();
        if (!analyzer.analyze(source, error)) {
            std::cerr << "Expected success for " << source << ": " << error << '\n';
            return 1;
        }
    }
    for (const char* source : {"declare 256 as byte", "declare 255 plus 1 as byte",
                               "declare 0 - 1 as byte", "declare 1 as word", "put 1 into ax"}) {
        error.clear();
        if (analyzer.analyze(source, error) || error.empty()) {
            std::cerr << "Expected an error for " << source << '\n';
            return 1;
        }
    }
    return 0;
}
