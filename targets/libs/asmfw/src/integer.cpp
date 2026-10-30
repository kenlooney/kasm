#include "integer.hpp"
#include <limits>

bool parse_integer(const std::string& text, std::uint64_t& value) {
    unsigned base = 10;
    std::size_t pos = 0;
    if (text.size() >= 2 && text[0] == '0') {
        char prefix = text[1];
        if (prefix == 'x' || prefix == 'X') { base = 16; pos = 2; }
        else if (prefix == 'b' || prefix == 'B') { base = 2; pos = 2; }
        else if (prefix == 'o' || prefix == 'O') { base = 8; pos = 2; }
    }
    if (pos == text.size()) return false;
    std::uint64_t result = 0;
    for (; pos < text.size(); ++pos) {
        char ch = text[pos];
        unsigned digit;
        if (ch >= '0' && ch <= '9') digit = ch - '0';
        else if (ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
        else return false;
        if (digit >= base) return false;
        if (result > (std::numeric_limits<std::uint64_t>::max() - digit) / base)
            return false;
        result = result * base + digit;
    }
    value = result;
    return true;
}
