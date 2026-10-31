#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <iostream>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    std::vector<KasmStatement> statements;
    std::string error;
    if (!parser.parse_statements("// moves\nput 0b1010 into ax\nmove (2 plus 3) to [16 + 4]\n2 + 3\n", statements, error) || statements.size() != 3) {
        std::cerr << "Expected three statements: " << error << '\n';
        return 1;
    }
    const auto& reg = statements[0];
    const auto& mem = statements[1];
    if (reg.type() != STATEMENT_TYPE_MOVE || reg.destination_kind() != DestinationKind::Register ||
        reg.destination_register() != "ax" || reg.source().nodes[reg.source().root].value != 10 ||
        mem.destination_kind() != DestinationKind::Memory || mem.source().nodes[mem.source().root].kind != "+" ||
        mem.destination_address().nodes[mem.destination_address().root].kind != "+" ||
        statements[2].type() != STATEMENT_TYPE_EXPRESSION) {
        std::cerr << "Incorrect parsed statement\n";
        return 1;
    }
    for (const char* text : {"put 1 ax", "put into ax", "put 1 into", "put 1 into []", "put 1 into [2 +]", "put 1 into ax bx"}) {
        error.clear();
        if (parser.parse_statements(text, statements, error) || error.empty() || statements.size() != 3) {
            std::cerr << "Expected error without changing output: " << text << '\n';
            return 1;
        }
    }
    std::vector<Expression> expressions;
    if (parser.parse_statements("put 1 into ax", expressions, error)) {
        std::cerr << "Expression-only output must reject move statements\n";
        return 1;
    }
    return 0;
}
