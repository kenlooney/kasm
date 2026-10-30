#include "kasmparser.hpp"
#include "integer.hpp"

KasmParser::KasmParser(const Lexer& lexer) : Parser(lexer) {}

bool KasmParser::parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error) {
    position_ = 0;
    tokens_ = tokens;
    return run(value, error);
}
bool KasmParser::number(std::uint64_t& value, std::string& error) {
    if(position_ >= tokens_.size() || tokens_[position_].kind != "integer") {
        error = "Expected an integer";
        return false;
    }
    if(!parse_integer(tokens_[position_].text, value)) {
        error = "Invalid integer: " + tokens_[position_].text;
        return false;
    }
    ++position_;
    return true;

}