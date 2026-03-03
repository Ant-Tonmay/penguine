#include "vm/vm.h"
#include <iostream>

namespace vm {

void VM::push(Value v){
    stack.push_back(v);
}

Value VM::pop(){
    Value v = stack.back();
    stack.pop_back();
    return v;
}


inline double as_double(const Value& v) {
    if (std::holds_alternative<double>(v)) return std::get<double>(v);
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1.0 : 0.0;
    if (std::holds_alternative<int>(v)) return (double)std::get<int>(v);
    return 0.0;
}

inline int as_int(const Value& v) {
    if (std::holds_alternative<int>(v)) return std::get<int>(v);
    if (std::holds_alternative<double>(v)) return (int)std::get<double>(v);
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1 : 0;
    return 0;
}

inline bool as_bool(const Value& v) {
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    if (std::holds_alternative<int>(v)) return std::get<int>(v) != 0;
    if (std::holds_alternative<double>(v)) return std::get<double>(v) != 0.0;
    return false;
}

void VM::run(FunctionObject* script){

    frames.push_back({script, 0, 0});

    while(true){
        auto& frame = frames.back();
        uint8_t instruction = frame.function->chunk.code[frame.ip++];

        switch(instruction){

            case OP_CONSTANT:{
                uint8_t idx = frame.function->chunk.code[frame.ip++];
                push(frame.function->chunk.constants[idx]);
                break;
            }
            case OP_TRUE: push(true); break;
            case OP_FALSE: push(false); break;
            case OP_NULL: push(std::monostate{}); break;

            case OP_GET_LOCAL: {
                uint8_t slot = frame.function->chunk.code[frame.ip++];
                push(stack[frame.base + slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = frame.function->chunk.code[frame.ip++];
                stack[frame.base + slot] = stack.back();
                break;
            }
            case OP_GET_GLOBAL: {
                uint8_t name_idx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[name_idx]);
                push(globals[name]);
                break;
            }
            case OP_SET_GLOBAL: {
                uint8_t name_idx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[name_idx]);
                globals[name] = stack.back();
                break;
            }
            case OP_POP: {
                pop();
                break;
            }

            case OP_ADD:
            case OP_PLUS_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a+b);
                break;
            }
            case OP_SUB:
            case OP_MINUS_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a-b);
                break;
            }
            case OP_MUL:
            case OP_MULTIPLY_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a*b);
                break;
            }
            case OP_DIV:
            case OP_DIVIDE_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a/b);
                break;
            }
            case OP_MOD:
            case OP_MODULO_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a%b));
                break;
            }

            case OP_BITWISE_AND:
            case OP_BITWISE_AND_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a & b));
                break;
            }
            case OP_BITWISE_OR:
            case OP_BITWISE_OR_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a | b));
                break;
            }
            case OP_XOR:
            case OP_XOR_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a ^ b));
                break;
            }
            case OP_LEFT_SHIFT:
            case OP_LEFT_SHIFT_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a << b));
                break;
            }
            case OP_RIGHT_SHIFT:
            case OP_RIGHT_SHIFT_EQUAL: {
                int b = as_int(pop());
                int a = as_int(pop());
                push((double)(a >> b));
                break;
            }

            case OP_LOGICAL_AND:
            case OP_LOGICAL_AND_EQUAL: {
                bool b = as_bool(pop());
                bool a = as_bool(pop());
                push(a && b);
                break;
            }
            case OP_LOGICAL_OR:
            case OP_LOGICAL_OR_EQUAL: {
                bool b = as_bool(pop());
                bool a = as_bool(pop());
                push(a || b);
                break;
            }

            case OP_GREATER: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a > b);
                break;
            }
            case OP_LESSER: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a < b);
                break;
            }
            case OP_GREATER_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a >= b);
                break;
            }
            case OP_LESSER_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a <= b);
                break;
            }
            case OP_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a == b);
                break;
            }
            case OP_NOT_EQUAL: {
                double b = as_double(pop());
                double a = as_double(pop());
                push(a != b);
                break;
            }

            case OP_NOT: {
                bool a = as_bool(pop());
                push(!a);
                break;
            }

            case OP_JUMP: {
                uint16_t offset = (frame.function->chunk.code[frame.ip] << 8) | frame.function->chunk.code[frame.ip + 1];
                frame.ip += 2;
                frame.ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = (frame.function->chunk.code[frame.ip] << 8) | frame.function->chunk.code[frame.ip + 1];
                frame.ip += 2;
                if (!as_bool(stack.back())) {
                    frame.ip += offset;
                }
                break;
            }

            case OP_LOOP: {
                uint16_t offset = (frame.function->chunk.code[frame.ip] << 8) | frame.function->chunk.code[frame.ip + 1];
                frame.ip += 2;
                frame.ip -= offset;
                break;
            }

            case OP_NEGATE:{
                double v = as_double(pop());
                push(-v);
                break;
            }

            case OP_PRINT:{
                auto v = pop();
                if (std::holds_alternative<bool>(v)) {
                    std::cout << (std::get<bool>(v) ? "true" : "false") << std::endl;
                } else if (std::holds_alternative<std::string>(v)) {
                    std::cout << std::get<std::string>(v) << std::endl;
                } else {
                    std::cout << as_double(v) << std::endl;
                }
                break;
            }

            case OP_CALL: {
                uint8_t argCount = frame.function->chunk.code[frame.ip++];
                // The stack looks like: [... callee arg0 arg1 ... argN]
                // callee is at stack[top - argCount - 1]
                Value calleeVal = stack[stack.size() - argCount - 1];

                if (!std::holds_alternative<FunctionObject*>(calleeVal)) {
                    std::cerr << "Runtime error: tried to call a non-function" << std::endl;
                    return;
                }

                FunctionObject* callee = std::get<FunctionObject*>(calleeVal);

                if (argCount != callee->arity) {
                    std::cerr << "Runtime error: expected " << callee->arity 
                              << " arguments but got " << (int)argCount << std::endl;
                    return;
                }

                // Push a new call frame
                // base points to the first argument (callee is at base - 1)
                size_t base = stack.size() - argCount;
                frames.push_back({callee, 0, base});
                break;
            }

            case OP_RETURN: {
                Value result = pop();

                // Get the current frame's base before popping it
                size_t base = frame.base;

                frames.pop_back();

                if (frames.empty()) {
                    // We returned from the top-level script
                    return;
                }

                // Discard the called function's locals, args, and the callee slot
                // Stack should be truncated to base - 1 (removing callee too)
                stack.resize(base - 1);

                // Push the return value
                push(result);
                break;
            }

            case OP_HALT:
                return;
        }
    }
}

}
