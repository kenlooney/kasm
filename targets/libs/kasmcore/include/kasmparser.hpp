#include "parser.hpp"
#include "lexer.hpp"
class KasmParser : public Parser {
public:
    explicit KasmParser(const Lexer& lexer);
    bool parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error) override;
    bool number(std::uint64_t& value, std::string& error) override;
};