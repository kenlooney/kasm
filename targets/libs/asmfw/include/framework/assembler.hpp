#pragma once

#include <memory>
#include <string>
#include <utility>

#include "lexer.hpp"
#include "parser.hpp"

class Assembler {
public:
    Assembler(std::string name, std::string source,
              std::unique_ptr<Lexer> lexer,
              std::unique_ptr<Parser> parser)
        : name_(std::move(name)),
          source_(std::move(source)),
          lexer_(std::move(lexer)),
          parser_(std::move(parser)) {}

private:
    std::string name_;
    std::string source_;
    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<Parser> parser_;
};