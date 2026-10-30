#pragma once
#include "span.hpp"
#include <string>

struct Token {
    std::string kind;
    Span span;
    std::string text;
};