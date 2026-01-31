#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <iostream>

using Value = std::variant<
    int,      
    bool,
    char,
    float,
    double,
    long,
    long long,
    std::monostate,
    std::string 
>;
