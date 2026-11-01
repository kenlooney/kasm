#pragma once
#include "framework/lexer.hpp"

class KasmLexer : public Lexer {
public:
    std::vector<Token> tokenize(const std::string& text) override;
};