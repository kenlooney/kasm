#pragma once
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
class Register {
public:
    explicit Register(std::string name, std::size_t width_bits) : name_(std::move(name)), width_bits_(width_bits) {}
    virtual ~Register() = default;
    const std::string& name() const { return name_; }
    const std::size_t& width_bits() const { return width_bits_; }
private:
    std::string name_;
    std::size_t width_bits_;
};

class RegisterFile {
public:
    explicit RegisterFile(std::vector<std::unique_ptr<Register>> registers) : registers(std::move(registers)) {
        if(!validate_registers()) {
            throw std::runtime_error("Invalid register file");
        }
    }
    virtual ~RegisterFile() = default;
    // Returns the number of registers in the register file.
    std::size_t size() const { return registers.size(); }
    // Returns the register at the specified index. Throws std::out_of_range if the index is invalid.
    Register& at(std::size_t index) {
        return *registers.at(index); 
    }
    // Returns the register at the specified index. Throws std::out_of_range if the index is invalid.
    const Register& at(std::size_t index) const { 
        return *registers.at(index); 
    }
    bool validate_registers() const {
        for (const auto& r : registers) {
            if (!r) {
                return false;
            }
        }
        return true;
    }
private:
    std::vector<std::unique_ptr<Register>> registers;
};