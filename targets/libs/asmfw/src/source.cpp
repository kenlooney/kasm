#include "source.hpp"
#include <fstream>
#include <sstream>

bool Source::load(const std::string& path, Source& result) {
    std::ifstream input(path);
    if(!input) return false;
    std::ostringstream contents;
    contents << input.rdbuf();
    result.name = path;
    result.text = contents.str();
    return true;
}