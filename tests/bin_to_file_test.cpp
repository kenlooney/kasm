#include "source.hpp"
#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "kasmsemantic.hpp"
#include "kasmwriter.hpp"
#include <iostream>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: <source-file> <output-file>\n";
        return 1;
    }
    Source source;
    if (!Source::load(argv[1], source)) {
        std::cerr << "Could not open: " << argv[1] << "\n";
        return 1;
    }
    const char* output_file = argv[2];
    std::vector<std::uint8_t> bytes;
    KasmLexer lexer;
    KasmParser parser(lexer);
    std::vector<KasmStatement> statements;
    std::string error;
    KasmSemanticAnalyzer analyzer(parser);
    if (!analyzer.analyze(source.text, error)) {
        std::cerr << "Semantic error: " << error << "\n";
        return 1;
    }
    if (!parser.parse_statements(source.text, statements, error)) {
        std::cerr << "Parse error: " << error << "\n";
        return 1;
    }
    for (const auto& statement : statements) {
        const Expression& expression = statement.source();
        std::uint64_t value = 0;
        if (!parser.evaluate(expression, expression.root, value, error)) {
            std::cerr << "Evaluation error: " << error << "\n";
            return 1;
        }
        bytes.push_back(static_cast<std::uint8_t>(value));
    }
    if (bytes != std::vector<std::uint8_t>{5, 0x20}) {
        std::cerr << "Unexpected declaration bytes\n";
        return 1;
    }
    std::cout << "Bytes:";
    for (std::uint8_t byte : bytes) {
        std::cout << ' ' << static_cast<unsigned int>(byte);
    }
    std::cout << '\n';
    
    //NEW!
    KasmWriter writer(output_file, KasmEndianness::Little);
    writer.set_buffer(bytes);
    // Little endian test
    if (!writer.write_to_file(bytes)) {
        std::cerr << "Error writing to output file: " << output_file << "\n";
        return 1;
    }
   

    return 0;
}