#pragma once

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "parser/ast.h"
#include "symbol_table/value.h"


inline void printValue(const Value& val) {
    if (std::holds_alternative<int>(val)) {
        std::cout << std::get<int>(val);
    } else if (std::holds_alternative<bool>(val)) {
        std::cout << (std::get<bool>(val) ? "true" : "false");
    } else if (std::holds_alternative<std::string>(val)) {
        std::cout << std::get<std::string>(val);
    } else if (std::holds_alternative<double>(val)) {
        std::cout << std::get<double>(val);
    } else if (std::holds_alternative<float>(val)) {
        std::cout << std::get<float>(val);
    } else if (std::holds_alternative<long>(val)) {
        std::cout << std::get<long>(val);
    } else if (std::holds_alternative<long long>(val)) {
        std::cout << std::get<long long>(val);
    } else if (std::holds_alternative<char>(val)) {
        std::cout << std::get<char>(val);
    } else if (std::holds_alternative<ArrayObject*>(val)) {
        auto arr = std::get<ArrayObject*>(val);
        std::cout << "[Array length=" << arr->length << "]";
    } else if (std::holds_alternative<std::monostate>(val)) {
        std::cout << "null";
    }
}

