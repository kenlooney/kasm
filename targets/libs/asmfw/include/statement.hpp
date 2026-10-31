#pragma once
#include <string>
#include <utility>

 
class Statement {
public:
    explicit Statement(std::string name) : name_(std::move(name)) {}
    virtual ~Statement() = default;
private:
    std::string name_;

};