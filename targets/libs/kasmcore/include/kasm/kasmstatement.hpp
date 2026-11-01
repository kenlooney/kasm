#pragma once

#include "framework/statement.hpp"
#include "framework/parser.hpp"

typedef enum {
    STATEMENT_TYPE_UNKNOWN = 0,
    STATEMENT_TYPE_DECLARE_BYTE = 1, // "db"
    STATEMENT_TYPE_MOVE_IMM16 = 3,
    STATEMENT_TYPE_MOVE_REG = 4,
    STATEMENT_TYPE_EXPRESSION = 5,
    STATEMENT_TYPE_MOVE = 6,
} StatementType;

enum class DestinationKind { None, Register, Memory };

class KasmStatement : public Statement {
public:
    explicit KasmStatement(std::string name, StatementType type) : Statement(std::move(name)), type_(type) {}
    ~KasmStatement() override = default;
    StatementType type() const { return type_; }
    const Expression& source() const { return source_; }
    DestinationKind destination_kind() const { return destination_kind_; }
    const std::string& destination_register() const { return destination_register_; }
    const Expression& destination_address() const { return destination_address_; }
    void set_source(Expression source) { source_ = std::move(source); }
    void set_register_destination(std::string name) { destination_kind_ = DestinationKind::Register; destination_register_ = std::move(name); }
    void set_memory_destination(Expression address) { destination_kind_ = DestinationKind::Memory; destination_address_ = std::move(address); }
    

private:
    StatementType type_ = STATEMENT_TYPE_UNKNOWN;
    Expression source_;
    DestinationKind destination_kind_ = DestinationKind::None;
    std::string destination_register_;
    Expression destination_address_;
};
