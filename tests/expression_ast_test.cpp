#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <iostream>

int main() {
    KasmLexer lexer;
    KasmParser parser(lexer);
    Expression tree;
    std::string error;
    if (!parser.parse_expression(lexer.tokenize("2 plus 3 times 4"), tree, error)) {
        std::cerr << error << "\n";
        return 1;
    }
    if (tree.nodes.size() != 5 || tree.root >= tree.nodes.size()) return 1;
    const Node& add = tree.nodes[tree.root];
    if (add.kind != "+" || add.left >= tree.nodes.size() || add.right >= tree.nodes.size()) return 1;
    const Node& two = tree.nodes[add.left];
    const Node& multiply = tree.nodes[add.right];
    if (two.kind != "number" || two.value != 2 || multiply.kind != "*" ||
        multiply.left >= tree.nodes.size() || multiply.right >= tree.nodes.size()) return 1;
    if (tree.nodes[multiply.left].value != 3 || tree.nodes[multiply.right].value != 4) return 1;
    Expression saved = tree;
    if (!parser.parse_expression(lexer.tokenize("1 - 2"), tree, error) || tree.nodes[tree.root].kind != "-") return 1;
    std::uint64_t value = 0;
    if (!parser.evaluate(saved, saved.root, value, error) || value != 14) {
        std::cerr << "Expected saved tree to evaluate to 14: " << error << "\n";
        return 1;
    }
    if (parser.evaluate(saved, saved.nodes.size(), value, error)) return 1;
    if (parser.parse_expression(lexer.tokenize("1 +"), tree, error)) return 1;
    return 0;
}
