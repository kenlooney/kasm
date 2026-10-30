#include "source.hpp"
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
    int line = 1;
    int column = 1;
    for (char ch : source.text) {
        if (ch == '\n') {
            ++line;
            column = 1;
        } else {
            std::cout << line << ":" << column << " " << ch << "\n";
            ++column;
        }
    }
    return 0;
}
