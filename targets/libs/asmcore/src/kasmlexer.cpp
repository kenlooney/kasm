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

            ++cursor;
            tokens.push_back({"character", {begin, cursor}, text.substr(begin, 1)});
        }
        return tokens;
}
