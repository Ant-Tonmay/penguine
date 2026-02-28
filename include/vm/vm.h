#pragma once
#include "chunk.h"
#include <vector>
#include <unordered_map>

namespace vm {

struct CallFrame {
    Chunk* chunk;
    size_t ip;
    size_t base;
};

class VM {
public:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;

    void push(Value v);
    Value pop();

    void run(Chunk& chunk);
};

}
