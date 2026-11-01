#include "kasm/kasmlexer.hpp"
#include "kasm/kasmparser.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    std::vector<KasmStatement> statements;
    std::string error;
    if (!parser.parse_statements("declare 10 as byte\nset (2 plus 3) being byte\ndeclare 256 as byte\n", statements, error) || statements.size() != 3) {
        std::cerr << "Expected three declarations: " << error << '\n';
        return 1;
    }
    if (statements[0].name() != "declare" || statements[1].name() != "declare" ||
        statements[0].type() != STATEMENT_TYPE_DECLARE_BYTE ||
        statements[0].source().nodes[statements[0].source().root].value != 10 ||
        statements[1].source().nodes[statements[1].source().root].kind != "+" ||
        statements[2].source().nodes[statements[2].source().root].value != 256) {
        std::cerr << "Incorrect declaration AST\n";
        return 1;
    }
    for (const char* input : {"declare 10 byte", "declare as byte", "declare 10 as", "declare 10 as word",
                              "declare 10 as byte extra", "declare 2 + as byte"}) {
        error.clear();
        if (parser.parse_statements(input, statements, error) || error.empty() || statements.size() != 3) {
            std::cerr << "Expected a parse error for: " << input << '\n';
            return 1;
        }
    }
    return 0;
}
