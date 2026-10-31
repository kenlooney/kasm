#include "kasmparser.hpp"
#include "integer.hpp"

KasmParser::KasmParser(const Lexer& lexer, const std::string& filename)
    : Parser(lexer), ops_filename(filename), aliases_loaded_(false) {
    reload_operator_aliases();
}

bool KasmParser::reload_operator_aliases() {
    aliases_loaded_ = load_operator_aliases(ops_filename);
    if(!has_operator_aliases("+"))
        add_operator_alias({"+", {"+","add","plus"}});
    if(!has_operator_aliases("-"))
        add_operator_alias({"-", {"-","sub","minus"}});
    if(!has_operator_aliases("*"))
        add_operator_alias({"*", {"*","mul","times"}});
    return aliases_loaded_;
}

bool KasmParser::save_operator_aliases() const {
    return Parser::save_operator_aliases(ops_filename);
}

bool KasmParser::operator_aliases_loaded() const {
    return aliases_loaded_;
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
