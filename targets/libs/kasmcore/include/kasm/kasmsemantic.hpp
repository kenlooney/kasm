#pragma once
#include "framework/semantic.hpp"
#include "kasm/kasmparser.hpp"
#include "kasm/kasmstatement.hpp"
class KasmSemanticAnalyzer : public SemanticAnalyzer {
public:
    explicit KasmSemanticAnalyzer(KasmParser& parser) : SemanticAnalyzer(parser), kasm_parser_(parser) {}
    bool analyze(const std::string& source, std::string& error) override;
    const KasmParser& parser() const { return static_cast<const KasmParser&>(SemanticAnalyzer::parser()); }
private:
   KasmParser& kasm_parser_;
   bool validate_statement(const KasmStatement& statement, std::string& error);
};
