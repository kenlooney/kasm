#include "parser.hpp"
#include "lexer.hpp"
#include <string>

class KasmParser : public Parser {
public:
    explicit KasmParser(const Lexer& lexer, const std::string& ops_filename = "ops.txt");
    bool reload_operator_aliases();
    using Parser::save_operator_aliases;
    bool save_operator_aliases() const;
    bool operator_aliases_loaded() const;
    bool parse_statements(const std::string& source, std::vector<Expression>& statements, std::string& error);
    bool number(std::uint64_t& value, std::string& error) override;
private:
    std::string ops_filename;
    bool aliases_loaded_;

};
