#include "framework/source.hpp"
#include "kasm/kasmlexer.hpp"
#include "kasm/kasmparser.hpp"
#include "kasm/kasmsemantic.hpp"
#include "kasm/kasmwriter.hpp"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: kasm <source-file> <output-file>\n";
        return 1;
    }

    Source source;
    if (!Source::load(argv[1], source)) {
        std::cerr << "Could not open source file: " << argv[1] << '\n';
        return 1;
    }

    KasmLexer lexer;
    KasmParser parser(lexer);
    KasmSemanticAnalyzer analyzer(parser);
    std::string error;
    if (!analyzer.analyze(source.text, error)) {
        std::cerr << "Semantic error: " << error << '\n';
        return 1;
    }

    std::vector<KasmStatement> statements;
    if (!parser.parse_statements(source.text, statements, error)) {
        std::cerr << "Parse error: " << error << '\n';
        return 1;
    }
    std::vector<std::uint8_t> bytes;
    for (const auto& statement : statements) {
        const Expression& expression = statement.source();
        std::uint64_t value = 0;
        if (!parser.evaluate(expression, expression.root, value, error)) {
            std::cerr << "Evaluation error: " << error << '\n';
            return 1;
        }
        bytes.push_back(static_cast<std::uint8_t>(value));
    }

    KasmWriter writer(argv[2], KasmEndianness::Little);
    if (!writer.write_to_file(bytes)) {
        std::cerr << "Could not write output file: " << argv[2] << '\n';
        return 1;
    }
    return 0;
}
