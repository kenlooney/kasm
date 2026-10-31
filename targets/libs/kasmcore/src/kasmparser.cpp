#include "kasmparser.hpp"
#include "integer.hpp"
#include "kasmlexer.hpp"
#include <algorithm>

static std::string parse_mnemonic(const std::vector<Token> &line_tokens)
{
    if (line_tokens.empty())
        return "";
    return line_tokens[0].text;
}
bool KasmParser::is_as_alias(const std::string &mnemonic)
{
    for (const auto &keywords : keyword_aliases())
    {
        if (keywords.symbol != "as")
            continue;
        if (mnemonic == keywords.symbol)
            return true;
        for (const auto &value : keywords.values)
        {
            if (value == mnemonic)
                return true;
        }
    }
    return false;
}
bool KasmParser::is_move_alias(const std::string &mnemonic)
{
    for (const auto &keywords : keyword_aliases())
    {
        if (keywords.symbol != "mov")
            continue;
        if (mnemonic == keywords.symbol)
            return true;
        for (const auto &value : keywords.values)
        {
            if (value == mnemonic)
                return true;
        }
    }
    return false;
}

bool KasmParser::is_declare_alias(const std::string &mnemonic)
{
   for (const auto &keywords : keyword_aliases())
    {
        if (keywords.symbol != "declare")
            continue;
        if (mnemonic == keywords.symbol)
            return true;
        for (const auto &value : keywords.values)
        {
            if (value == mnemonic)
                return true;
        }
    }
    return false;
}

bool KasmParser::is_into_alias(const std::string &mnemonic)
{
    for (const auto &keywords : keyword_aliases())
    {
        if (keywords.symbol != "into")
            continue;
        if (mnemonic == keywords.symbol)
            return true;
        for (const auto &value : keywords.values)
        {
            if (value == mnemonic)
                return true;
        }
    }
    return false;
}

bool KasmParser::parse_statements(const std::string &source, std::vector<Expression> &statements, std::string &error)
{
    std::vector<KasmStatement> parsed;
    if (!parse_statements(source, parsed, error)) return false;
    std::vector<Expression> expressions;
    for (const auto &statement : parsed) {
        if (statement.type() != STATEMENT_TYPE_EXPRESSION) {
            error = "Non-expression statements require KasmStatement output";
            return false;
        }
        expressions.push_back(statement.source());
    }
    statements = std::move(expressions);
    return true;
}

bool KasmParser::parse_statements(const std::string &source, std::vector<KasmStatement> &statements, std::string &error)
{
    KasmLexer lexer;
    const auto tokens = lexer.tokenize(source);
    std::vector<KasmStatement> parsed;
    std::vector<Token> line_tokens;
    std::size_t line = 1, statement_line = 1, scanned = 0;

    auto finish_line = [&]()
    {
        if (line_tokens.empty())
            return true;

        // 1. Parse mnemonic
        std::string mnemonic = parse_mnemonic(line_tokens);
        
        
        
        if (mnemonic.empty())
        {
            error = "Line " + std::to_string(statement_line) + ": Missing mnemonic";
            return false;
        }
        if (is_declare_alias(mnemonic))
        {
            auto separator = std::find_if(line_tokens.begin() + 1, line_tokens.end(),
                [this](const Token& token) { return is_as_alias(token.text); });
            if (separator == line_tokens.end()) {
                error = "Line " + std::to_string(statement_line) + ": Expected 'as' or 'being'";
                return false;
            }
            if (separator == line_tokens.begin() + 1) {
                error = "Line " + std::to_string(statement_line) + ": Missing declaration value";
                return false;
            }
            if (separator + 1 == line_tokens.end()) {
                error = "Line " + std::to_string(statement_line) + ": Missing type after 'as'";
                return false;
            }
            if (separator + 2 != line_tokens.end()) {
                error = "Line " + std::to_string(statement_line) + ": Unexpected tokens after declaration type";
                return false;
            }
            const std::string& type = (separator + 1)->text;
            if (type != "byte") {
                error = "Line " + std::to_string(statement_line) + ": Unsupported declaration type: " + type;
                return false;
            }
            std::vector<Token> value_tokens(line_tokens.begin() + 1, separator);
            Expression value;
            if (!parse_expression(value_tokens, value, error)) {
                error = "Line " + std::to_string(statement_line) + ": " + error;
                return false;
            }
            KasmStatement statement("declare", STATEMENT_TYPE_DECLARE_BYTE);
            statement.set_source(std::move(value));
            parsed.push_back(std::move(statement));
            line_tokens.clear();
            return true;
        }
        
        if (is_move_alias(mnemonic))
        {
            auto separator = std::find_if(line_tokens.begin() + 1, line_tokens.end(),
                [this](const Token& token) { return is_into_alias(token.text); });
            if (separator == line_tokens.end()) {
                error = "Line " + std::to_string(statement_line) + ": Expected 'into' or 'to'";
                return false;
            }
            if (separator == line_tokens.begin() + 1 || separator + 1 == line_tokens.end()) {
                error = "Line " + std::to_string(statement_line) + ": Expected source and destination";
                return false;
            }
            std::vector<Token> source_tokens(line_tokens.begin() + 1, separator);
            Expression source_expression;
            if (!parse_expression(source_tokens, source_expression, error))
            {
                error = "Line " + std::to_string(statement_line) + ": " + error;
                return false;
            }
            KasmStatement statement("mov", STATEMENT_TYPE_MOVE);
            statement.set_source(std::move(source_expression));
            auto destination = separator + 1;
            if (destination + 1 == line_tokens.end() && destination->kind == "identifier") {
                statement.set_register_destination(destination->text);
            } else if (destination->text == "[" && line_tokens.back().text == "]" &&
                       line_tokens.end() - destination > 2) {
                std::vector<Token> address_tokens(destination + 1, line_tokens.end() - 1);
                Expression address;
                if (!parse_expression(address_tokens, address, error)) {
                    error = "Line " + std::to_string(statement_line) + ": Invalid memory address: " + error;
                    return false;
                }
                statement.set_memory_destination(std::move(address));
            } else {
                error = "Line " + std::to_string(statement_line) + ": Expected register or [address] destination";
                return false;
            }
            parsed.push_back(std::move(statement));
            line_tokens.clear();
            return true;
        }

        Expression expression;
        if (!parse_expression(line_tokens, expression, error))
        {
            error = "Line " + std::to_string(statement_line) + ": " + error;
            return false;
        }
        KasmStatement statement("expression", STATEMENT_TYPE_EXPRESSION);
        statement.set_source(std::move(expression));
        parsed.push_back(std::move(statement));
        line_tokens.clear();
        return true;
    };

    for (const auto &token : tokens)
    {
        while (scanned < token.span.begin)
        {
            if (source[scanned] == '\r' || source[scanned] == '\n')
            {
                if (!finish_line())
                    return false;
                if (source[scanned] == '\r' && scanned + 1 < token.span.begin && source[scanned + 1] == '\n')
                    ++scanned;
                ++line;
            }
            ++scanned;
        }
        if (line_tokens.empty())
            statement_line = line;
        line_tokens.push_back(token);
        scanned = token.span.end;
    }
    if (!finish_line())
        return false;
    statements = std::move(parsed);
    return true;
}

KasmParser::KasmParser(const Lexer &lexer) : Parser(lexer)
{
    // Add operator aliases
    add_operator_alias({"+", {"+", "add", "plus"}});
    add_operator_alias({"-", {"-", "sub", "minus"}});
    add_operator_alias({"*", {"*", "mul", "times"}});

    // Add keyword aliases
    add_keyword_alias({"mov", {"mov", "move", "put"}});
    add_keyword_alias({"into", {"into", "to"}});
    add_keyword_alias({"declare", {"declare", "set"}});
    add_keyword_alias({"as", {"as","being"}});
    // Examples:
    // mov 0b1010 into ax
    // move 0b1010 to ax
    // put 0b1010 into ax
}

bool KasmParser::number(std::uint64_t &value, std::string &error)
{
    if (position_ >= tokens_.size() || tokens_[position_].kind != "integer")
    {
        error = "Expected an integer";
        return false;
    }
    if (!parse_integer(tokens_[position_].text, value))
    {
        error = "Invalid integer: " + tokens_[position_].text;
        return false;
    }
    ++position_;
    return true;
}
