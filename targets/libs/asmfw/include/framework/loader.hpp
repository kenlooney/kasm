#pragma once
#include "framework/cpu.hpp"
#include <string>

class Loader {
public:
    Loader() = default;
    ~Loader();
    Loader(const Loader&) = delete;
    Loader& operator=(const Loader&) = delete;
    bool open(const std::string& path, std::string& error);
    Cpu* cpu() const { return instance;}
private:
    void* library = nullptr;
    Cpu* instance = nullptr;
    void (*destroy)(Cpu*) = nullptr;
};