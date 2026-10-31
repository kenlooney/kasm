#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "token.hpp"
#include "integer.hpp"
#include <limits>
#include "lexer.hpp"
#include "aliastable.hpp"
#include <string>

struct Node {
    std::string kind;
    std::uint64_t value;
    std::size_t left;
    std::size_t right;
};

struct Expression {
    std::vector<Node> nodes;
    std::size_t root = 0;
};

class Parser {
    public:
    explicit Parser(const Lexer& lexer);
    virtual ~Parser() = default;
    bool parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error);
    bool parse_expression(const std::vector<Token>& tokens, Expression& expression, std::string& error);
    virtual bool number(std::uint64_t& value, std::string& error) = 0;
    bool primary(std::size_t& node, std::string& error);
    bool product(std::size_t& node, std::string& error);
    bool run(std::string& error);
    bool sum(std::size_t& node, std::string& error);
    bool has_operator_aliases(const std::string& operator_symbol) const;
    bool has_keyword_aliases(const std::string& keyword_symbol) const;
    const std::vector<Alias>& operator_aliases() const { return operator_alias_table_.aliases(); }
    const std::vector<Alias>& keyword_aliases() const { return keyword_alias_table_.aliases(); }
    bool add_operator_alias(const Alias& alias);
    bool add_keyword_alias(const Alias& alias);
    bool valid_operator(const std::string& op) const;
    bool valid_keyword(const std::string& keyword) const;
    bool add_alias_to_operator(const std::string& operator_symbol, const std::string& alias);
    bool add_alias_to_keyword(const std::string& keyword_symbol, const std::string& alias);
    protected:
    std::size_t position_ = 0;
    std::vector<Token> tokens_;
    Expression tree_;
    AliasTable operator_alias_table_;
    AliasTable keyword_alias_table_;
    bool evaluate(std::size_t node, std::uint64_t& value, std::string& error) const;
};
