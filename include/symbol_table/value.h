#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <iostream>


struct ArrayObject; 
using Value = std::variant<
    int,      
    bool,
    char,
    float,
    double,
    long,
    long long,
    ArrayObject*,
    std::monostate,
    std::string 
>;
struct ArrayObject {
    bool isFixed;
    size_t length;
    size_t capacity;
    Value* data;
};
