#include "kasmsemantic.hpp"

bool KasmSemanticAnalyzer::validate_statement(const KasmStatement &statement, std::string &error)
{
    bool is_valid = false;
    std::string name = statement.name();
    if (name == "declare") {
        if (statement.type() != STATEMENT_TYPE_DECLARE_BYTE) {
            error = "Invalid statement type for declare";
            return false;
        }
       
        is_valid = true;
        
    }
    return is_valid;
}
bool KasmSemanticAnalyzer::analyze(const std::string &source, std::string &error)
{
    // Implement the semantic analysis for Kasm here.
    // Return true if analysis is successful, false otherwise.
    return true;
}