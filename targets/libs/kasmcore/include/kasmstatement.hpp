#pragma once

#include "statement.hpp"

typedef enum {
    STATEMENT_TYPE_UNKNOWN = 0,
    STATEMENT_TYPE_MOVE_IMM16 = 1,
    STATEMENT_TYPE_MOVE_REG = 2,
} StatementType;

class KasmStatement : public Statement {
public:
    explicit KasmStatement(std::string name, StatementType type) : Statement(std::move(name)), type_(type) {}
    ~KasmStatement() override = default;
    StatementType type() const { return type_; }
    

private:
    StatementType type_ = STATEMENT_TYPE_UNKNOWN;
    std::string destination_;
    std::string source_;
};