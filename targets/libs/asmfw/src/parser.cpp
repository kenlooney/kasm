#include "parser.hpp"
#include <algorithm>

Parser::Parser(const Lexer &lexer) :
    tokens_(lexer.tokens()),
    operator_alias_table_()
{

}
bool Parser::run(std::string &error)
{
    tree_.nodes.clear();
    if(!sum(tree_.root, error)) return false;
    if (position_ != tokens_.size())
    {
        error = "Unexpected token:" + tokens_[position_].text;
        return false;
    }
    return true;
}
bool Parser::sum(std::size_t& node, std::string &error)
{
    if (!product(node, error))
        return false;
    while (position_ < tokens_.size() && (
    valid_operator("+") ||
    valid_operator("-")
))
    {
        const std::string op = valid_operator("+") ? "+" : "-";
        ++position_;
        std::size_t right = 0;
        if (!product(right, error))
            return false;
        tree_.nodes.push_back({op, 0, node, right});
        node = tree_.nodes.size() - 1;
    }
    return true;
}
bool Parser::product(std::size_t &node, std::string &error)
{
    if (!primary(node, error))
        return false;
    while (position_ < tokens_.size() && valid_operator("*"))
    {
        ++position_;
        std::size_t right = 0;
        if (!primary(right, error))
            return false;
        tree_.nodes.push_back({"*", 0, node, right});
        node = tree_.nodes.size() - 1;
    }
    return true;
}
bool Parser::primary(std::size_t &node, std::string &error)
{
    if (position_ < tokens_.size() && tokens_[position_].text == "(")
    {
        ++position_;
        if (!sum(node, error))
            return false;
        if (position_ >= tokens_.size() || tokens_[position_].text != ")")
        {
            error = "Expected ')'";
            return false;
        }
        ++position_;
        return true;
    }
    std::uint64_t value = 0;
    if (!number(value, error))
        return false;
    tree_.nodes.push_back({"number", value, 0, 0});
    node = tree_.nodes.size() - 1;
    return true;
}
bool Parser::parse_expression(const std::vector<Token>& tokens, std::uint64_t& value, std::string& error)
{
    Expression expression;
    if (!parse_expression(tokens, expression, error))
        return false;
    return evaluate(expression.root, value, error);
}

bool Parser::parse_expression(const std::vector<Token>& tokens, Expression& expression, std::string& error)
{
    position_ = 0;
    tokens_ = tokens;
    if (!run(error))
        return false;
    expression = tree_;
    return true;
}

bool Parser::evaluate(std::size_t index, std::uint64_t& value, std::string& error) const
{
    const Node& node = tree_.nodes[index];
    if (node.kind == "number") {
        value = node.value;
        return true;
    }
    std::uint64_t left = 0, right = 0;
    if (!evaluate(node.left, left, error) || !evaluate(node.right, right, error))
        return false;
    if (node.kind == "+") {
        if (left > std::numeric_limits<std::uint64_t>::max() - right) {
            error = "Addition overflow";
            return false;
        }
        value = left + right;
    } else if (node.kind == "-") {
        if (left < right) {
            error = "Subtraction underflow";
            return false;
        }
        value = left - right;
    } else if (node.kind == "*") {
        if (right != 0 && left > std::numeric_limits<std::uint64_t>::max() / right) {
            error = "Multiplication overflow";
            return false;
        }
        value = left * right;
    } else {
        error = "Unknown expression node: " + node.kind;
        return false;
    }
    return true;
}

bool Parser::has_operator_aliases(const std::string& operator_symbol) const
{
    const auto& aliases = operator_alias_table_.aliases();
    for (const auto& alias : aliases) {
        if (alias.symbol == operator_symbol) {
            return true;
        }
    }
    return false;
}
bool Parser::has_keyword_aliases(const std::string& keyword_symbol) const
{
    const auto& aliases = keyword_alias_table_.aliases();
    for (const auto& alias : aliases) {
        if (alias.symbol == keyword_symbol) {
            return true;
        }
    }
    return false;
}
bool Parser::add_operator_alias(const Alias& alias)
{
    operator_alias_table_.add_alias(alias);
    return true;
}
bool Parser::add_keyword_alias(const Alias& alias)
{
    keyword_alias_table_.add_alias(alias);
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
        if (alias.symbol == op) {
            if (std::find(alias.values.begin(), alias.values.end(), token_text) != alias.values.end())
                return true;
        }
    }
    return false;
}
bool Parser::valid_keyword(const std::string& keyword) const
{
    if (position_ >= tokens_.size())
        return false;
    const auto& token_text = tokens_[position_].text;
    if (token_text == keyword)
        return true;
    const auto& aliases = keyword_alias_table_.aliases();
    for (const auto& alias : aliases) {
        if (alias.symbol == keyword) {
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
        if (op_alias.symbol == operator_symbol) {
            operator_alias_table_.add_alias({operator_symbol, {alias}});
            return true;
        }
    }
    return false;
}
bool Parser::add_alias_to_keyword(const std::string& keyword_symbol, const std::string& alias)
{
    const auto& aliases = keyword_alias_table_.aliases();
    for (const auto& kw_alias : aliases) {
        if (kw_alias.symbol == keyword_symbol) {
            keyword_alias_table_.add_alias({keyword_symbol, {alias}});
            return true;
        }
    }
    return false;
}
