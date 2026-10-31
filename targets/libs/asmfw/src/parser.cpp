#include "parser.hpp"
#include <algorithm>

Parser::Parser(const Lexer &lexer) :
    tokens_(lexer.tokens()),
    operator_alias_table_()
{

}
bool Parser::run(std::uint64_t &value, std::string &error)
{
    if (!sum(value, error))
        return false;
    if (position_ != tokens_.size())
    {
        error = "Unexpected token:" + tokens_[position_].text;
        return false;
    }
    return true;
}
bool Parser::sum(std::uint64_t &value, std::string &error)
{
    if(!has_operator_aliases("+"))
    {
        add_operator_alias({"+", {"+"}});
    }
    if(!has_operator_aliases("-"))
    {
        add_operator_alias({"-", {"-"}});
    }

    if (!product(value, error))
        return false;
    while (position_ < tokens_.size() && (
    valid_operator("+") ||
    valid_operator("-")
))
    {
        const bool addition = valid_operator("+");
        ++position_;
        std::uint64_t right = 0;
        if (!product(right, error))
            return false;

        if(addition)
        {
            if (value > std::numeric_limits<std::uint64_t>::max() - right)
            {
                error = "Addition overflow";
                return false;
            }
            value += right;
        }
        else
        {
            if (value < right)
            {
                error = "Subtraction underflow";
                return false;
            }
            value -= right;
        }
    }
    return true;
}
bool Parser::product(std::uint64_t &value, std::string &error)
{
    if(!has_operator_aliases("*"))
    {
        add_operator_alias({"*", {"*"}});
    }

    if (!primary(value, error))
        return false;
    while (position_ < tokens_.size() && valid_operator("*"))
    {
        ++position_;
        std::uint64_t right = 0;
        if (!primary(right, error))
            return false;
        if (right != 0 && value > std::numeric_limits<std::uint64_t>::max() / right)
        {
            error = "Multiplication overflow";
            return false;
        }
        value *= right;
    }
    return true;
}
bool Parser::primary(std::uint64_t &value, std::string &error)
{
    if (position_ < tokens_.size() && tokens_[position_].text == "(")
    {
        ++position_;
        if (!sum(value, error))
            return false;
        if (position_ >= tokens_.size() || tokens_[position_].text != ")")
        {
            error = "Expected ')'";
            return false;
        }
        ++position_;
        return true;
    }
    return number(value, error);
}
bool Parser::parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error)
{
    position_ = 0;
    tokens_ = tokens;
    return run(value, error);
}

bool Parser::load_operator_aliases(const std::string& filename)
{
    return operator_alias_table_.load_from_file(filename);
}

bool Parser::save_operator_aliases(const std::string& filename) const
{
    return operator_alias_table_.save_to_file(filename);
}
bool Parser::has_operator_aliases(const std::string& operator_symbol) const
{
    const auto& aliases = operator_alias_table_.aliases();
    for (const auto& alias : aliases) {
        if (alias.operator_symbol == operator_symbol) {
            return true;
        }
    }
    return false;
}
bool Parser::add_operator_alias(const OperatorAlias& alias)
{
    operator_alias_table_.add_alias(alias);
    return true;
}
bool Parser::valid_operator(const std::string& op) const
{
    if (position_ >= tokens_.size())
        return false;
    const auto& token_text = tokens_[position_].text;
    if (token_text == op)
        return true;
    const auto& aliases = operator_alias_table_.aliases();
    for (const auto& alias : aliases) {
        if (alias.operator_symbol == op) {
            if (std::find(alias.values.begin(), alias.values.end(), token_text) != alias.values.end())
                return true;
        }
    }
    return false;
}
bool Parser::add_alias_to_operator(const std::string& operator_symbol, const std::string& alias)
{
    const auto& aliases = operator_alias_table_.aliases();
    for (const auto& op_alias : aliases) {
        if (op_alias.operator_symbol == operator_symbol) {
            operator_alias_table_.add_alias({operator_symbol, {alias}});
            return true;
        }
    }
    return false;
}
