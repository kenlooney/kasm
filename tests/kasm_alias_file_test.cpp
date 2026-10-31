#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

int main() {
    const auto filename = (std::filesystem::temp_directory_path() /
        ("kasm_alias_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".txt")).string();
    struct Cleanup {
        std::string filename;
        ~Cleanup() { std::error_code error; std::filesystem::remove(filename, error); }
    } cleanup{filename};

    KasmLexer lexer;
    KasmParser writer(lexer, filename);
    if (writer.operator_aliases_loaded()) {
        std::cerr << "Expected missing file to be reported\n";
        return 1;
    }
    writer.add_operator_alias({"+", {"add"}});
    writer.add_operator_alias({"*", {"mul"}});
    if (!writer.save_operator_aliases()) {
        std::cerr << "Could not save aliases\n";
        return 1;
    }

    KasmParser reader(lexer, filename);
    std::uint64_t value = 0;
    std::string error;
    if (!reader.operator_aliases_loaded() ||
        !reader.parse_expression(lexer.tokenize("2 add 3 mul 4"), value, error) || value != 14) {
        std::cerr << "Saved aliases did not load: " << error << "\n";
        return 1;
    }
    reader.add_operator_alias({"+", {"temporary"}});
    if (!reader.reload_operator_aliases() ||
        reader.parse_expression(lexer.tokenize("2 temporary 3"), value, error) ||
        !reader.parse_expression(lexer.tokenize("2 + 3"), value, error) || value != 5) {
        std::cerr << "Reload did not replace aliases or preserve built-in operators\n";
        return 1;
    }
    return 0;
}
