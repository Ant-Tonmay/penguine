#pragma once
#include "symbol_table/value.h"


struct BreakSignal {};
struct ContinueSignal {};
struct ReturnException {
    Value value;
};
