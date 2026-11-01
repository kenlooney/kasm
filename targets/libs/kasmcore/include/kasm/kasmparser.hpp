#pragma once
#include "framework/parser.hpp"
#include "framework/lexer.hpp"
#include "kasmstatement.hpp"
#include <string>

class KasmParser : public Parser {
public:
    explicit KasmParser(const Lexer& lexer);
    bool parse_statements(const std::string& source, std::vector<Expression>& statements, std::string& error);
    bool parse_statements(const std::string& source, std::vector<KasmStatement>& statements, std::string& error);
    
    bool number(std::uint64_t& value, std::string& error) override;
    bool is_move_alias(const std::string &mnemonic);
    bool is_declare_alias(const std::string &mnemonic);
    bool is_into_alias(const std::string &mnemonic);
    bool is_as_alias(const std::string &mnemonic);
    
};
