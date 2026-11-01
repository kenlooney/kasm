#include "framework/integer.hpp"

#include <iostream>
#include <limits>

int main() {
    const struct {
        const char* text;
        std::uint64_t expected;
    } cases[] = {
        {"0", 0},
        {"42", 42},
        {"0x2a", 42},
        {"0b101010", 42},
        {"0o52", 42},
        {"18446744073709551615", std::numeric_limits<std::uint64_t>::max()}
    };
    for (const auto& test : cases) {
        std::uint64_t value = 0;
        if (!parse_integer(test.text, value) || value != test.expected) {
            std::cerr << "Failed to parse " << test.text << "\n";
            return 1;
        }
        else
            std::cout << "Parsed " << test.text << " as " << value << "\n";
    }
    for (const char* text : {"0x", "0b2", "0o8", "12z", "18446744073709551616"}) {
        std::uint64_t value = 42;
        if (parse_integer(text, value) || value != 42) {
            std::cerr << "Invalid integer was not rejected unchanged: " << text << "\n";
            return 1;
        }
    }
    return 0;
}
