#include "parser.hpp"

Parser::Parser(const Lexer &lexer)
{
    tokens_ = lexer.tokens();
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
    if (!number(value, error))
        return false;
    while (position_ < tokens_.size() && tokens_[position_].text == "+")
    {
        ++position_;
        std::uint64_t right = 0;
        if (!number(right, error))
            return false;
        if (value > std::numeric_limits<std::uint64_t>::max() - right)
        {
            error = "Integer overflow";
            return false;
        }
        value += right;
    }
    return true;
}
