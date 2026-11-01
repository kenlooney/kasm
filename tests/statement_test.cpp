#include "kasm/kasmlexer.hpp"
#include "kasm/kasmparser.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    std::vector<Expression> statements;
    std::string error;
    const std::string source = "// One expression per line\r\n2 + 3 * 4 // first\r\n"
                               "/* comment\ncontinues */\n(2 + 3) * 4\n\n";
    if (!parser.parse_statements(source, statements, error) || statements.size() != 2) {
        std::cerr << "Expected two statements: " << error << "\n";
        return 1;
    }
    const auto& first = statements[0];
    const auto& second = statements[1];
    if (first.nodes[first.root].kind != "+" ||
        first.nodes[first.nodes[first.root].right].kind != "*" ||
        second.nodes[second.root].kind != "*" ||
        second.nodes[second.nodes[second.root].left].kind != "+") {
        std::cerr << "Incorrect statement trees or precedence\n";
        return 1;
    }
    if (parser.parse_statements("1 + 2\n3 +\n4", statements, error) ||
        error.find("Line 2:") != 0 || statements.size() != 2) {
        std::cerr << "Expected a line 2 error and unchanged output\n";
        return 1;
    }
    if (!parser.parse_statements("// comment only\n/* comment */", statements, error) || !statements.empty()) {
        std::cerr << "Expected no statements for comments\n";
        return 1;
    }
    return 0;
}
