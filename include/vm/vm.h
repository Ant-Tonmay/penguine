#pragma once
#include "chunk.h"
#include <vector>
#include <unordered_map>

namespace vm {

struct CallFrame {
    FunctionObject* function;
    size_t ip;
    size_t base;  // stack base for this call frame
};

class VM {
public:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;

    void push(Value v);
    Value pop();

    void run(FunctionObject* script);
};

}
