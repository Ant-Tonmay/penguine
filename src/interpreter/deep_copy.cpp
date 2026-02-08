#include "interpreter/interpreter.h"
#include <iostream>
#include <stdexcept>


Value Interpreter::deepCopyIfNeeded(const Value& v) {
    if (!std::holds_alternative<ArrayObject*>(v)) {
        return v; 
    }

    ArrayObject* src = std::get<ArrayObject*>(v);

    ArrayObject* dst = new ArrayObject;
    dst->isFixed  = src->isFixed;
    dst->length   = src->length;
    dst->capacity = src->capacity;
    dst->data     = new Value[dst->capacity];

    for (size_t i = 0; i < dst->length; i++) {
        dst->data[i] = deepCopyIfNeeded(src->data[i]);
    }

    return dst;
}
