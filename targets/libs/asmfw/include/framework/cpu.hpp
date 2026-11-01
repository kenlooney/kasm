#pragma once
#include <cstdint>
#include <string>
class Cpu {
public:
    Cpu() = default;
    virtual ~Cpu() = default;
    virtual bool encode_no_operand(const std::string& name, std::uint8_t& byte) const = 0;
    
};
