#pragma once
#include <vector>
#include "opcode.h"
#include "value.h"

namespace vm {

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;

    void write(uint8_t byte) {
        code.push_back(byte);
    }

    void write16(uint16_t value) {
        code.push_back((value >> 8) & 0xff);
        code.push_back(value & 0xff);
    }

    int addConstant(Value v) {
        constants.push_back(v);
        return constants.size() - 1;
    }
};

}
