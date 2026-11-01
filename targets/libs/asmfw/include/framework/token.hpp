#pragma once
#include "span.hpp"
#include <string>

struct Token {
    std::string kind;
    std::string text;
    Span span;
};