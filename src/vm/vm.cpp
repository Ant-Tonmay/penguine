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

inline std::string valueToString(const Value& val) {
    if (std::holds_alternative<int>(val)) {
        return std::to_string(std::get<int>(val));
    } else if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? "true" : "false";
    } else if (std::holds_alternative<std::string>(val)) {
        return std::get<std::string>(val);
    } else if (std::holds_alternative<double>(val)) {
        std::string str = std::to_string(std::get<double>(val));
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') str.pop_back();
        return str;
    } else if (std::holds_alternative<float>(val)) {
        std::string str = std::to_string(std::get<float>(val));
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') str.pop_back();
        return str;
    } else if (std::holds_alternative<long>(val)) {
        return std::to_string(std::get<long>(val));
    } else if (std::holds_alternative<long long>(val)) {
        return std::to_string(std::get<long long>(val));
    } else if (std::holds_alternative<char>(val)) {
        return std::string(1, std::get<char>(val));
    } else if (std::holds_alternative<ArrayObject*>(val)) {
        auto arr = std::get<ArrayObject*>(val);
        return "[Array length=" + std::to_string(arr->length) + "]";
    } else if (std::holds_alternative<std::monostate>(val)) {
        return "null";
    }
    return "";
}

Value deepCopyIfNeeded(const Value& v) {
    if (std::holds_alternative<ArrayObject*>(v)) {
        ArrayObject* original = std::get<ArrayObject*>(v);
        ArrayObject* copy = new ArrayObject();
        copy->length = original->length;
        copy->capacity = original->capacity;
        copy->isFixed = original->isFixed;
        copy->data = new Value[copy->capacity];
        for (size_t i = 0; i < copy->length; ++i) {
             copy->data[i] = deepCopyIfNeeded(original->data[i]);
        }
        return copy;
    }
    return v;
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
                Value bVal = pop();
                Value aVal = pop();
                
                if (std::holds_alternative<std::string>(aVal) || std::holds_alternative<std::string>(bVal)) {
                    std::string aStr = valueToString(aVal);
                    std::string bStr = valueToString(bVal);
                    push(aStr + bStr);
                } else {
                    double b = as_double(bVal);
                    double a = as_double(aVal);
                    push(a+b);
                }
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
                std::cout << valueToString(v);
                break;
            }
            
            case OP_PRINTLN:{
                auto v = pop();
                std::cout << valueToString(v) << std::endl;
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

            case OP_NEW_ARRAY: {
                uint8_t count = frame.function->chunk.code[frame.ip++];
                
                // Collect elements
                std::vector<Value> elements(count);
                for (int i = count - 1; i >= 0; i--) {
                    elements[i] = pop();
                }
                
                // Auto-unwrap single array elements
                if (count == 1 && std::holds_alternative<ArrayObject*>(elements[0])) {
                    push(elements[0]);
                    break;
                }
                
                ArrayObject* arr = new ArrayObject();
                arr->length = count;
                arr->capacity = count;
                arr->data = new Value[count];
                for (int i = 0; i < count; i++) {
                    arr->data[i] = elements[i];
                }
                push(arr);
                break;
            }

            case OP_INDEX_GET: {
                Value idxVal = pop();
                Value arrVal = pop();
                if (!std::holds_alternative<ArrayObject*>(arrVal)) {
                    std::cerr << "Runtime error: index operation expects an array." << std::endl;
                    return;
                }
                ArrayObject* arr = std::get<ArrayObject*>(arrVal);
                int i = as_int(idxVal);
                if (i < 0 || (size_t)i >= arr->length) {
                    std::cerr << "Runtime error: array index " << i << " out of bounds (length " << arr->length << ")." << std::endl;
                    return;
                }
                push(arr->data[i]);
                break;
            }

            case OP_INDEX_SET: {
                Value val = pop();
                Value idxVal = pop();
                Value arrVal = pop();
                if (!std::holds_alternative<ArrayObject*>(arrVal)) {
                    std::cerr << "Runtime error: index operation expects an array." << std::endl;
                    return;
                }
                ArrayObject* arr = std::get<ArrayObject*>(arrVal);
                int i = as_int(idxVal);
                if (i < 0 || (size_t)i >= arr->length) {
                    std::cerr << "Runtime error: array index " << i << " out of bounds (length " << arr->length << ")." << std::endl;
                    return;
                }
                arr->data[i] = val;
                break;
            }

            case OP_FIXED_ARRAY: {
                uint8_t argCount = frame.function->chunk.code[frame.ip++];
                Value initVal = std::monostate{};
                if (argCount == 2) {
                    initVal = pop();
                    // Auto-unwrap single-element array for initVal matching interpreter
                    if (std::holds_alternative<ArrayObject*>(initVal)) {
                        ArrayObject* initArr = std::get<ArrayObject*>(initVal);
                        if (initArr->length == 1) {
                            initVal = initArr->data[0];
                        }
                    }
                }
                int size = as_int(pop());
                if (size < 0) {
                    std::cerr << "Runtime error: fixed() size cannot be negative." << std::endl;
                    return;
                }
                ArrayObject* arr = new ArrayObject();
                arr->isFixed = true;
                arr->length = size;
                arr->capacity = size;
                arr->data = new Value[size];
                for (int i = 0; i < size; ++i) {
                    arr->data[i] = deepCopyIfNeeded(initVal);
                }
                push(arr);
                break;
            }

            case OP_ARRAY_PUSH: {
                Value val = pop();
                Value arrVal = pop();
                if (!std::holds_alternative<ArrayObject*>(arrVal)) {
                    std::cerr << "Runtime error: push expects an array." << std::endl;
                    return;
                }
                ArrayObject* arr = std::get<ArrayObject*>(arrVal);
                if (arr->isFixed) {
                    std::cerr << "Runtime error: Cannot push to fixed array." << std::endl;
                    return;
                }
                if (arr->length == arr->capacity) {
                    size_t newCap = arr->capacity == 0 ? 4 : arr->capacity * 2;
                    Value* newData = new Value[newCap];
                    for (size_t i = 0; i < arr->length; ++i) newData[i] = arr->data[i];
                    if (arr->data) delete[] arr->data;
                    arr->data = newData;
                    arr->capacity = newCap;
                }
                arr->data[arr->length++] = val;
                push(std::monostate{});
                break;
            }

            case OP_HALT:
                return;
        }
    }
}

}
