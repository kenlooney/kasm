#pragma once
#include <string>

struct Source {
    std::string name;
    std::string text;
    static bool load(const std::string& path, Source& result);
};