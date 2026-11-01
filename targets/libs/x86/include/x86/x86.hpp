#pragma once
#include <cstdint>
#include "framework/cpu.hpp"
#include <string>

class X86Encoder {
public:
    void encode_one(const char* name, std::uint8_t& byte);
};
class X86 : public Cpu {
public:
    X86() : encoder() {}
    ~X86() override = default;
    bool encode_no_operand(const std::string& name, std::uint8_t& byte) const override;
    const X86Encoder& get_encoder() const { return encoder; }
    void encode_one(const char* name, std::uint8_t& byte);
protected:
    X86Encoder encoder;

};
