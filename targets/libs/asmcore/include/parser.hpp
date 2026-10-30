#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "token.hpp"
#include "integer.hpp"
#include <limits>
#include "lexer.hpp"
class Parser {
    public:
    explicit Parser(const Lexer& lexer);
    virtual ~Parser() = default;
    virtual bool parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error) = 0;
    virtual bool number(std::uint64_t& value, std::string& error) = 0;
    bool run(std::uint64_t& value, std::string& error);
    bool sum(std::uint64_t& value, std::string& error);
    

    protected:
    std::size_t position_ = 0;
    std::vector<Token> tokens_;
};