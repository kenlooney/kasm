#include "framework/source.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: <source-file>\n";
        return 1;
    }
    Source source;
    if (!Source::load(argv[1], source)) {
        std::cerr << "Could not open: " << argv[1] << "\n";
        return 1;
    }
    std::cout << "File: " << source.name << "\n" << source.text;
    return 0;
}
