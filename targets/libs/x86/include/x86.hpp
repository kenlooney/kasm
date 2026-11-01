#pragma once
#include <cstdint>
#include "cpu.hpp"

class X86 : public Cpu {
public:
    X86() = default;
    ~X86() override = default;
};