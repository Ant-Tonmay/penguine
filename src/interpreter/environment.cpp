#include "interpreter/environment.h"

Environment::Environment(Environment* parent) : parent(parent) {}

void Environment::define(const std::string& name, const Value& value) {
    values[name] = value;
}

void Environment::assign(const std::string& name, const Value& value) {
    if (values.count(name)) {
        values[name] = value;
        return;
    }

    if (parent) {
        parent->assign(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable '" + name + "'.");
}

Value Environment::get(const std::string& name) {
    if (values.count(name)) {
        return values[name];
    }

    if (parent) {
        return parent->get(name);
    }

    throw std::runtime_error("Undefined variable '" + name + "'.");
}
