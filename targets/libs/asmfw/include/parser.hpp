#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "token.hpp"
#include "integer.hpp"
#include <limits>
#include "lexer.hpp"
#include "operatoralias.hpp"
#include <string>

class Parser {
    public:
    explicit Parser(const Lexer& lexer);
    virtual ~Parser() = default;
    bool parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error);
    virtual bool number(std::uint64_t& value, std::string& error) = 0;
    bool primary(std::uint64_t& value, std::string& error);
    bool product(std::uint64_t& value, std::string& error);
    bool run(std::uint64_t& value, std::string& error);
    bool sum(std::uint64_t& value, std::string& error);
    bool load_operator_aliases(const std::string& filename);
    bool save_operator_aliases(const std::string& filename) const;
    bool has_operator_aliases(const std::string& operator_symbol) const;
    const std::vector<OperatorAlias>& operator_aliases() const { return operator_alias_table_.aliases(); }
    bool add_operator_alias(const OperatorAlias& alias);
    bool valid_operator(const std::string& op) const;
    bool add_alias_to_operator(const std::string& operator_symbol, const std::string& alias);
    protected:
    std::size_t position_ = 0;
    std::vector<Token> tokens_;
    OperatorAliasTable operator_alias_table_;
};
