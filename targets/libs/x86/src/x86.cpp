#include "x86/x86.hpp"

#include <string>
#include <cstring>

bool X86::encode_no_operand(const std::string& name, std::uint8_t& byte) const {
    if (name != "NOP" && name != "nop") return false;
    byte = 0x90;
    return true;
}

void X86::encode_one(const char* name, std::uint8_t& byte) {
    encoder.encode_one(name, byte);
}

void X86Encoder::encode_one(const char* name, std::uint8_t& byte) {
    if (strcmp(name, "NOP") == 0) {
        byte = 0x90; // Example encoding for NOP
    } 
}
 
