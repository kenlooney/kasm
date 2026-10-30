#pragma once
#include "token.hpp"
#include <vector>

class Lexer {
public:
    virtual ~Lexer() = default;
    virtual std::vector<Token> tokenize(const std::string& text) = 0;
};