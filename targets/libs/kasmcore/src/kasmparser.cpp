#include "kasmparser.hpp"
#include "integer.hpp"
#include "kasmlexer.hpp"

bool KasmParser::parse_statements(const std::string& source, std::vector<Expression>& statements, std::string& error) {
    KasmLexer lexer;
    const auto tokens = lexer.tokenize(source);
    std::vector<Expression> parsed;
    std::vector<Token> line_tokens;
    std::size_t line = 1, statement_line = 1, scanned = 0;

    auto finish_line = [&]() {
        if (line_tokens.empty()) return true;
        Expression expression;
        if (!parse_expression(line_tokens, expression, error)) {
            error = "Line " + std::to_string(statement_line) + ": " + error;
            return false;
        }
        parsed.push_back(std::move(expression));
        line_tokens.clear();
        return true;
    };

    for (const auto& token : tokens) {
        while (scanned < token.span.begin) {
            if (source[scanned] == '\r' || source[scanned] == '\n') {
                if (!finish_line()) return false;
                if (source[scanned] == '\r' && scanned + 1 < token.span.begin && source[scanned + 1] == '\n') ++scanned;
                ++line;
            }
            ++scanned;
        }
        if (line_tokens.empty()) statement_line = line;
        line_tokens.push_back(token);
        scanned = token.span.end;
    }
    if (!finish_line()) return false;
    statements = std::move(parsed);
    return true;
}

KasmParser::KasmParser(const Lexer& lexer) : Parser(lexer) {
    // Add operator aliases
    add_operator_alias({"+", {"+", "add", "plus"}});
    add_operator_alias({"-", {"-", "sub", "minus"}});
    add_operator_alias({"*", {"*", "mul", "times"}});

    // Add keyword aliases
    add_keyword_alias({"mov", {"mov","move", "put"}});
    add_keyword_alias({"into", {"into", "to"}});
    // Examples:
    // mov 0b1010 into ax
    // move 0b1010 to ax
    // put 0b1010 into ax
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
