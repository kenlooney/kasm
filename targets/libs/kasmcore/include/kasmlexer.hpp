#pragma once
#include "lexer.hpp"

class KasmLexer : public Lexer {
public:
    std::vector<Token> tokenize(const std::string& text) override;
};