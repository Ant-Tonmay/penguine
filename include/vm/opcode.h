#pragma once

namespace vm {

enum OpCode {
    OP_CONSTANT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_GREATER,
    OP_LESSER,
    OP_GREATER_EQUAL,
    OP_LESSER_EQUAL,
    OP_NOT,
    OP_PRINT,
    OP_HALT,
    OP_NOT_EQUAL,
    OP_TRUE,
    OP_FALSE,
    OP_NULL,
    OP_LEFT_SHIFT,
    OP_RIGHT_SHIFT,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_XOR,
    OP_NEGATE,
    OP_RETURN
};

}
