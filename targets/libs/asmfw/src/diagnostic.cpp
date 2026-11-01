#include "framework/diagnostic.hpp"
#include <iostream>

void report(const std::string& file, int line, int column,
            const std::string& message) {
    std::cerr << file << ":" << line << ":" << column
              << ": error: " << message << "\n";
}