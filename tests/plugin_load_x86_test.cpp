#include "framework/loader.hpp"
#include <cstdint>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: plugin_load_x86_test <plugin-path>\n";
        return 1;
    }
    std::string error;
    Loader missing;
    if (missing.open(std::string(argv[1]) + ".missing", error) || error.empty()) {
        std::cerr << "Expected a missing plugin to fail\n";
        return 1;
    }

    Loader loader;
    error.clear();
    if (!loader.open(argv[1], error) || !loader.cpu()) {
        std::cerr << "Could not load x86 plugin: " << error << '\n';
        return 1;
    }
    std::uint8_t byte = 0;
    if (!loader.cpu()->encode_no_operand("nop", byte) || byte != 0x90) {
        std::cerr << "Expected NOP to encode as 0x90\n";
        return 1;
    }
    if (loader.cpu()->encode_no_operand("unknown", byte)) {
        std::cerr << "Expected unknown instruction to fail\n";
        return 1;
    }
    std::cout << "Loaded x86 plugin and encoded NOP as 0x90\n";
    return 0;
}
