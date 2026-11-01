#pragma once

#include "parser.hpp"


class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(Parser& parser) : parser_(parser) {}
    virtual ~SemanticAnalyzer() = default;
    virtual bool analyze(const std::string& source, std::string& error) = 0;
    const Parser& parser() const { return parser_; }

private:
    Parser& parser_;
};