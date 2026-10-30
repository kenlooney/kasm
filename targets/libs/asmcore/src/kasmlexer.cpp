#include "kasmlexer.hpp"


std::vector<Token> KasmLexer::tokenize(const std::string& text) {
        std::vector<Token> tokens;
        std::size_t cursor = 0;

        
        while (cursor < text.size()) {
            char ch = text[cursor];

            if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                ++cursor;
                continue;
            }
            std::size_t begin = cursor;

            // Handle identifiers
            if (letter(ch)) {
                ++cursor;
                while (cursor < text.size() && (letter(text[cursor]) || digit(text[cursor]))) {
                    ++cursor;
                }
                tokens.push_back({"identifier", {begin, cursor}, text.substr(begin, cursor - begin)});
                continue;
            }
            // Handle numbers

            // Handle binary numbers
            if (ch == '0' && cursor + 1 < text.size() && (text[cursor + 1] == 'b' || text[cursor + 1] == 'B')) {
                cursor += 2;
                while (cursor < text.size() && (text[cursor] == '0' || text[cursor] == '1')) {
                    ++cursor;
                }
                tokens.push_back({"integer", {begin, cursor}, text.substr(begin, cursor - begin)});
                continue;
            }
            // Handle hexadecimal numbers
            if (ch == '0' && cursor + 1 < text.size() && (text[cursor + 1] == 'x' || text[cursor + 1] == 'X')) {
                cursor += 2;
                while (cursor < text.size() && ((text[cursor] >= '0' && text[cursor] <= '9') || (text[cursor] >= 'a' && text[cursor] <= 'f') || (text[cursor] >= 'A' && text[cursor] <= 'F'))) {
                    ++cursor;
                }
                tokens.push_back({"integer", {begin, cursor}, text.substr(begin, cursor - begin)});
                continue;
            }
            // Handle octal numbers
            if (ch == '0' && cursor + 1 < text.size() && (text[cursor + 1] >= '0' && text[cursor + 1] <= '7')) {
                cursor += 1;
                while (cursor < text.size() && (text[cursor] >= '0' && text[cursor] <= '7')) {
                    ++cursor;
                }
                tokens.push_back({"integer", {begin, cursor}, text.substr(begin, cursor - begin)});
                continue;
            }
            // Handle decimal numbers
            if (digit(ch)) {
                ++cursor;
                while (cursor < text.size() && digit(text[cursor])) {
                    ++cursor;
                }
                tokens.push_back({"integer", {begin, cursor}, text.substr(begin, cursor - begin)});
                continue;
            }


            ++cursor;
            tokens.push_back({"character", {begin, cursor}, text.substr(begin, 1)});
        }
        return tokens;
}
