#include "x86/x86.hpp"
#include <iostream>

int main() {
    X86 cpu;
    std::uint8_t byte;
    cpu.encode_one("NOP", byte);
    std::cout << "NOP encoded as: 0x" << std::hex << static_cast<unsigned int>(byte) << "\n";
  
    return 0;
}