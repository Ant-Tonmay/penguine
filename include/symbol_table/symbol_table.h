#pragma once

#include <unordered_map>
#include <string>
#include <stdexcept>

#include "value.h"

class SymbolTable {
public:
    explicit SymbolTable(SymbolTable* parent = nullptr);

    void define(const std::string& name, const Value& value);
    void assign(const std::string& name, const Value& value);
    Value get(const std::string& name) const;

private:
    SymbolTable* parent;
    std::unordered_map<std::string, Value> values;
};
