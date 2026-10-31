#include "parser.hpp"
#include "lexer.hpp"
#include <string>

class KasmParser : public Parser {
public:
    explicit KasmParser(const Lexer& lexer);
    bool parse_statements(const std::string& source, std::vector<Expression>& statements, std::string& error);
    bool number(std::uint64_t& value, std::string& error) override;
};
