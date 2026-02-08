#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <iostream>
#include "parser/ast.h"
struct FunctionObject;
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
    FunctionObject*,
    std::string 
>;
struct ArrayObject {
    bool isFixed;
    size_t length;
    size_t capacity;
    Value* data;      // Dynamic array of Values
    int refCount;     // Simple ref counting if we want to manage memory manually

    ArrayObject() : isFixed(false), length(0), capacity(0), data(nullptr), refCount(0) {}
    ~ArrayObject() {
        if (data) delete[] data;
    }
};
struct FunctionObject {
    Function* astNode; 
};