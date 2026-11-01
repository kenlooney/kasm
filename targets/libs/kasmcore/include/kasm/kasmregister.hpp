#include "register.hpp"

class KasmRegister : public Register {
public:
    explicit KasmRegister(std::string name, std::size_t width_bits) : Register(std::move(name), width_bits) {}
    virtual ~KasmRegister() = default;
};

class KasmRegisterFile : public RegisterFile {
public:
    explicit KasmRegisterFile(std::vector<std::unique_ptr<Register>> registers) : RegisterFile(std::move(registers)) {}
    virtual ~KasmRegisterFile() = default;
};