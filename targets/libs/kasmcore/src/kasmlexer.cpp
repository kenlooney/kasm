#include "kasm/kasmlexer.hpp"


std::vector<Token> KasmLexer::tokenize(const std::string& text) {
        std::vector<Token> tokens;
        std::size_t cursor = 0;

        
        while (cursor < text.size()) {
            char ch = text[cursor];

            if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                ++cursor;
                continue;
            }
            // Single-line modern comments
            if (ch == '/' && cursor + 1 < text.size() && text[cursor + 1] == '/') {
                cursor += 2;
                while (cursor < text.size() && text[cursor] != '\n') {
                    ++cursor;
                }
                continue;
            }
            // Handle multi-line comments
            if (ch == '/' && cursor + 1 < text.size() && text[cursor + 1] == '*') {
                cursor += 2;
                while (cursor + 1 < text.size() && !(text[cursor] == '*' && text[cursor + 1] == '/')) {
                    ++cursor;
                }
                if (cursor + 1 < text.size()) {
                    cursor += 2;
                }
                continue;
            }

            // Record the beginning of the current token
            std::size_t begin = cursor;

            // Handle identifiers
            if (letter(ch)) {
                ++cursor;
                while (cursor < text.size() && (letter(text[cursor]) || digit(text[cursor]))) {
                    ++cursor;
                }
                tokens.push_back({"identifier", text.substr(begin, cursor - begin), {begin, cursor}});
                continue;
            }
            // Handle numbers
             if (digit(ch)) {
            ++cursor;
            while (cursor < text.size() &&
                   (letter(text[cursor]) || digit(text[cursor]))) ++cursor;
            tokens.push_back({"integer", text.substr(begin, cursor - begin),
                              {begin, cursor}});
            continue;
        }
         

            ++cursor;
            tokens.push_back({"punctuation", text.substr(begin, 1), {begin, cursor}});
        }
        return tokens;
}
