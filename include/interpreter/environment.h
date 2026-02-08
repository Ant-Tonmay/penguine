#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include "interpreter/runtime_value.h"

class Environment {
public:
    Environment(Environment* parent = nullptr);
    ~Environment() = default;

    // Define a variable in the current scope
    void define(const std::string& name, const Value& value);

    // Assign to an existing variable (searches parent scopes)
    void assign(const std::string& name, const Value& value);

    // Retrieve a variable (searches parent scopes)
    Value get(const std::string& name);

    // Create a child environment
    // Note: Caller manages memory of the new environment
    // In a fuller implementation, we might use shared_ptr for Environments
    Environment* getParent() const { return parent; }

private:
    Environment* parent;
    std::unordered_map<std::string, Value> values;
};
