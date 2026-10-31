#include "kasmsemantic.hpp"

bool KasmSemanticAnalyzer::validate_statement(const KasmStatement &statement, std::string &error)
{
    if (statement.name() == "declare") {
        const Expression& expr = statement.source();
        // Evaluate the expression to ensure it is valid for a declare statement
        std::uint64_t value = 0;
        if (!parser().evaluate(expr, expr.root, value, error))
            return false;


        // Reject the declaration if the expression is not valid
        switch (statement.type()) {
            case STATEMENT_TYPE_DECLARE_BYTE:
                if (value > 0xFF) {
                    error = "Declaration value exceeds byte size";
                    return false;
                }
                break;
            default:
                error = "Unsupported statement type for declare";
                return false;
        }
       
        return true;
    }
    error = "Semantic checking is not implemented for statement: " + statement.name();
    return false;
}
bool KasmSemanticAnalyzer::analyze(const std::string &source, std::string &error)
{
    std::vector<KasmStatement> statements;
    if (!kasm_parser_.parse_statements(source, statements, error))
        return false;
    for (const auto& statement : statements) {
        if (!validate_statement(statement, error))
            return false;
    }
    return true;
}
