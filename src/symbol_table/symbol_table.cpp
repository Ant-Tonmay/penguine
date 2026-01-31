#include "symbol_table/symbol_table.h"

SymbolTable::SymbolTable(SymbolTable* parent)
    : parent(parent) {}

void SymbolTable::define(const std::string& name, const Value& value) {
    values[name] = value;
}

Value SymbolTable::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }

    if (parent != nullptr) {
        return parent->get(name);
    }

    throw std::runtime_error("Undefined variable: " + name);
}

void SymbolTable::assign(const std::string& name, const Value& value) {
    auto it = values.find(name);
    if (it != values.end()) {
        it->second = value;
        return;
    }

    if (parent != nullptr) {
        parent->assign(name, value);
        return;
    }

    throw std::runtime_error("Assignment to undefined variable: " + name);
}
