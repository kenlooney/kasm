#pragma once
#include "semantic.hpp"
#include "kasmparser.hpp"
#include "kasmstatement.hpp"
class KasmSemanticAnalyzer : public SemanticAnalyzer {
public:
    explicit KasmSemanticAnalyzer(KasmParser& parser) : SemanticAnalyzer(parser) {}
    bool analyze(const std::string& source, std::string& error) override;
private:
   bool validate_statement(const KasmStatement& statement, std::string& error);
};