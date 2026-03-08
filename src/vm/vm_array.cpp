#include "vm/vm.h"

#include "vm/utils/value_utils.h"

#include <iostream>
#include <vector>

namespace vm {

bool VM::handleArrayOp(CallFrame& frame, uint8_t instruction) {
    switch (instruction) {
        case OP_NEW_ARRAY: {
            uint8_t count = frame.function->chunk.code[frame.ip++];
            std::vector<Value> elements(count);
            for (int i = count - 1; i >= 0; i--) {
                elements[i] = pop();
            }

            if (count == 1 && std::holds_alternative<ArrayObject*>(elements[0])) {
                push(elements[0]);
                return true;
            }

            ArrayObject* arr = new ArrayObject();
            arr->length = count;
            arr->capacity = count;
            arr->data = new Value[count];
            for (int i = 0; i < count; i++) {
                arr->data[i] = elements[i];
            }
            push(arr);
            return true;
        }

        case OP_INDEX_GET: {
            Value idxValue = pop();
            Value arrValue = pop();
            if (!std::holds_alternative<ArrayObject*>(arrValue)) {
                std::cerr << "Runtime error: index operation expects an array." << std::endl;
                return false;
            }
            ArrayObject* arr = std::get<ArrayObject*>(arrValue);
            int idx = asInt(idxValue);
            if (idx < 0 || static_cast<size_t>(idx) >= arr->length) {
                std::cerr << "Runtime error: array index " << idx
                          << " out of bounds (length " << arr->length << ")." << std::endl;
                return false;
            }
            push(arr->data[idx]);
            return true;
        }

        case OP_INDEX_SET: {
            Value value = pop();
            Value idxValue = pop();
            Value arrValue = pop();
            if (!std::holds_alternative<ArrayObject*>(arrValue)) {
                std::cerr << "Runtime error: index operation expects an array." << std::endl;
                return false;
            }
            ArrayObject* arr = std::get<ArrayObject*>(arrValue);
            int idx = asInt(idxValue);
            if (idx < 0 || static_cast<size_t>(idx) >= arr->length) {
                std::cerr << "Runtime error: array index " << idx
                          << " out of bounds (length " << arr->length << ")." << std::endl;
                return false;
            }
            arr->data[idx] = value;
            return true;
        }

        case OP_FIXED_ARRAY: {
            uint8_t argCount = frame.function->chunk.code[frame.ip++];
            Value initValue = std::monostate{};
            if (argCount == 2) {
                initValue = pop();
                if (std::holds_alternative<ArrayObject*>(initValue)) {
                    ArrayObject* initArr = std::get<ArrayObject*>(initValue);
                    if (initArr->length == 1) {
                        initValue = initArr->data[0];
                    }
                }
            }

            int size = asInt(pop());
            if (size < 0) {
                std::cerr << "Runtime error: fixed() size cannot be negative." << std::endl;
                return false;
            }

            ArrayObject* arr = new ArrayObject();
            arr->isFixed = true;
            arr->length = size;
            arr->capacity = size;
            arr->data = new Value[size];
            for (int i = 0; i < size; ++i) {
                arr->data[i] = deepCopyIfNeeded(initValue);
            }
            push(arr);
            return true;
        }

        case OP_ARRAY_PUSH: {
            Value value = pop();
            Value arrValue = pop();
            if (!std::holds_alternative<ArrayObject*>(arrValue)) {
                std::cerr << "Runtime error: push expects an array." << std::endl;
                return false;
            }

            ArrayObject* arr = std::get<ArrayObject*>(arrValue);
            if (arr->isFixed) {
                std::cerr << "Runtime error: Cannot push to fixed array." << std::endl;
                return false;
            }

            if (arr->length == arr->capacity) {
                size_t newCap = arr->capacity == 0 ? 4 : arr->capacity * 2;
                Value* newData = new Value[newCap];
                for (size_t i = 0; i < arr->length; ++i) {
                    newData[i] = arr->data[i];
                }
                if (arr->data) delete[] arr->data;
                arr->data = newData;
                arr->capacity = newCap;
            }

            arr->data[arr->length++] = value;
            push(std::monostate{});
            return true;
        }

        case OP_ARRAY_LENGTH: {
            Value arrValue = pop();
            if (!std::holds_alternative<ArrayObject*>(arrValue)) {
                std::cerr << "Runtime error: length() expects an array." << std::endl;
                return false;
            }
            ArrayObject* arr = std::get<ArrayObject*>(arrValue);
            push(static_cast<double>(arr->length));
            return true;
        }

        default:
            return false;
    }
}

}  // namespace vm
