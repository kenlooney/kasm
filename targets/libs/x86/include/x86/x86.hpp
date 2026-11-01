#pragma once
#include <cstdint>
#include "framework/cpu.hpp"

class X86 : public Cpu {
public:
    X86() = default;
    ~X86() override = default;
};