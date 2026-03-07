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
    if (std::holds_alternative<int64_t>(v)) return (double)std::get<int64_t>(v);
    return 0.0;
}

inline int as_int(const Value& v) {
    if (std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v);
    if (std::holds_alternative<double>(v)) return (int)std::get<double>(v);
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1 : 0;
    return 0;
}

inline bool as_bool(const Value& v) {
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    if (std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v) != 0;
    if (std::holds_alternative<double>(v)) return std::get<double>(v) != 0.0;
    return false;
}

inline std::string valueToString(const Value& val) {
    if (std::holds_alternative<int64_t>(val)) {
        return std::to_string(std::get<int64_t>(val));
    } else if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? "true" : "false";
    } else if (std::holds_alternative<std::string>(val)) {
        return std::get<std::string>(val);
    } else if (std::holds_alternative<double>(val)) {
        std::string str = std::to_string(std::get<double>(val));
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') str.pop_back();
        return str;
    } else if (std::holds_alternative<char>(val)) {
        return std::string(1, std::get<char>(val));
    } else if (std::holds_alternative<ArrayObject*>(val)) {
        auto arr = std::get<ArrayObject*>(val);
        return "[Array length=" + std::to_string(arr->length) + "]";
    } else if (std::holds_alternative<ClassObject*>(val)) {
        auto cls = std::get<ClassObject*>(val);
        return "<class " + cls->name + ">";
    } else if (std::holds_alternative<InstanceObject*>(val)) {
        auto inst = std::get<InstanceObject*>(val);
        return "<" + inst->klass->name + " instance>";
    } else if (std::holds_alternative<BoundMethod*>(val)) {
        auto bm = std::get<BoundMethod*>(val);
        std::string mName = !bm->methods.empty() ? bm->methods[0]->name : "unknown";
        return "<bound method " + mName + ">";
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

static bool checkAccess(ClassObject* owner, ClassObject* context, AccessModifier access) {
    if (access == AccessModifier::PUBLIC) return true;
    if (access == AccessModifier::PRIVATE) {
        return context == owner;
    }
    if (access == AccessModifier::PROTECTED) {
        if (!context) return false;
        ClassObject* curr = context;
        while (curr != nullptr) {
            if (curr == owner) return true;
            curr = curr->parent;
        }
        curr = owner;
        while (curr != nullptr) {
            if (curr == context) return true;
            curr = curr->parent;
        }
        return false;
    }
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

                if (std::holds_alternative<ClassObject*>(calleeVal)) {
                    ClassObject* klass = std::get<ClassObject*>(calleeVal);
                    InstanceObject* instance = new InstanceObject(klass);
                    
                    
                    if (klass->methods.count(klass->name)) {
                        auto& initMethods = klass->methods[klass->name];
                        FunctionObject* matchingInit = nullptr;
                        for (auto* func : initMethods) {
                            if (argCount == func->arity - 1) { // -1 for `this`
                                matchingInit = func;
                                break;
                            }
                        }
                        
                        if (matchingInit) {
                            stack[stack.size() - argCount - 1] = instance;
                            size_t base = stack.size() - argCount - 1;
                            frames.push_back({matchingInit, 0, base});
                            break;
                        }
                        std::cerr << "Runtime error: no matching constructor for " << (int)argCount << " arguments." << std::endl;
                        return;
                    } else if (argCount != 0) {
                        std::cerr << "Runtime error: expected 0 arguments for default constructor but got " << (int)argCount << std::endl;
                        return;
                    }
                    
                    for (int i=0; i < argCount; i++) pop();
                    pop(); // pop class
                    push(instance);
                    break;
                }
                
                if (std::holds_alternative<BoundMethod*>(calleeVal)) {
                    BoundMethod* bound = std::get<BoundMethod*>(calleeVal);
                    
                    stack[stack.size() - argCount - 1] = bound->instance;
                    
                    FunctionObject* matchingMethod = nullptr;
                    for (auto* func : bound->methods) {
                        if (argCount == func->arity - 1) {
                            matchingMethod = func;
                            break;
                        }
                    }
                    
                    if (!matchingMethod) {
                        std::cerr << "Runtime error: expected method arguments did not match any overloaded method." << std::endl;
                        return;
                    }
                    
                    size_t base = stack.size() - argCount - 1;
                    frames.push_back({matchingMethod, 0, base});
                    break;
                }

                if (!std::holds_alternative<FunctionObject*>(calleeVal)) {
                    std::cerr << "Runtime error: tried to call a non-function" << std::endl;
                    return;
                }

                FunctionObject* callee = std::get<FunctionObject*>(calleeVal);

                if (argCount != callee->arity) {
                    std::cerr << "Runtime error: in function " << callee->name << " expected " << callee->arity 
                              << " arguments but got " << (int)argCount << std::endl;
                    return;
                }

                // Push a new call frame
                // base points to the callee, which is always at stack.size() - argCount - 1
                size_t base = stack.size() - argCount - 1;
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

                stack.resize(base);

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

            case OP_ARRAY_LENGTH: {
                Value arrVal = pop();
                if (!std::holds_alternative<ArrayObject*>(arrVal)) {
                    std::cerr << "Runtime error: length() expects an array." << std::endl;
                    return;
                }
                ArrayObject* arr = std::get<ArrayObject*>(arrVal);
                push((double)arr->length);
                break;
            }

            case OP_CLASS: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                ClassObject* klass = new ClassObject(name);
                push(klass);
                break;
            }

            case OP_METHOD: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                uint8_t modifier = frame.function->chunk.code[frame.ip++];
                
                Value methodVal = pop();
                Value klassVal = stack.back(); // peek
                
                if (!std::holds_alternative<ClassObject*>(klassVal)) {
                    std::cerr << "Runtime error: OP_METHOD expects class on stack." << std::endl;
                    return;
                }
                
                ClassObject* klass = std::get<ClassObject*>(klassVal);
                FunctionObject* func = std::get<FunctionObject*>(methodVal);
                func->ownerClass = klass;
                
                bool overridden = false;
                for (auto& existingMethod : klass->methods[name]) {
                    if (existingMethod->arity == func->arity) {
                        existingMethod = func;
                        overridden = true;
                        break;
                    }
                }
                if (!overridden) {
                    klass->methods[name].push_back(func);
                }
                
                klass->methodAccess[name] = static_cast<AccessModifier>(modifier);
                break;
            }

            case OP_GET_PROPERTY: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                Value objVal = pop();
                
                if (!std::holds_alternative<InstanceObject*>(objVal)) {
                    std::cerr << "Runtime error: OP_GET_PROPERTY expects an instance. Got: " << valueToString(objVal) << std::endl;
                    return;
                }
                
                InstanceObject* instance = std::get<InstanceObject*>(objVal);
                ClassObject* contextClass = frame.function->isMethod ? frame.function->ownerClass : nullptr;

                if (instance->fields.count(name) || instance->klass->fields.count(name)) {
                    AccessModifier access = AccessModifier::PUBLIC;
                    if (instance->klass->fields.count(name)) access = instance->klass->fields[name];
                    
                    if (!checkAccess(instance->klass, contextClass, access)) {
                        std::cerr << "Runtime error: Access denied to field '" << name << "'." << std::endl;
                        return;
                    }
                    if (instance->fields.count(name)) {
                        push(instance->fields[name]);
                    } else {
                        push(std::monostate{}); 
                    }
                } else if (instance->klass->methods.count(name)) {
                    AccessModifier access = instance->klass->methodAccess[name];
                    if (!checkAccess(instance->klass, contextClass, access)) {
                        std::cerr << "Runtime error: Access denied to method '" << name << "'." << std::endl;
                        return;
                    }
                    
                    std::vector<FunctionObject*> mVec = instance->klass->methods[name];
                    BoundMethod* bound = new BoundMethod(instance, mVec);
                    push(bound);
                } else {
                    std::cerr << "Runtime error: Undefined property '" << name << "'." << std::endl;
                    return;
                }
                break;
            }
            
            case OP_SET_PROPERTY: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                Value value = pop();
                Value objVal = pop();
                
                if (!std::holds_alternative<InstanceObject*>(objVal)) {
                    std::cerr << "Runtime error: OP_SET_PROPERTY expects an instance." << std::endl;
                    return;
                }
                
                InstanceObject* instance = std::get<InstanceObject*>(objVal);
                ClassObject* contextClass = frame.function->isMethod ? frame.function->ownerClass : nullptr;
                
                if (instance->klass->fields.count(name)) {
                    AccessModifier access = instance->klass->fields[name];
                    if (!checkAccess(instance->klass, contextClass, access)) {
                        std::cerr << "Runtime error: Access denied to field '" << name << "'." << std::endl;
                        return;
                    }
                }
                
                instance->fields[name] = value;
                push(value); 
                break;
            }

            case OP_GET_PROPERTY_OR_GLOBAL: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                Value objVal = pop();
                
                if (std::holds_alternative<InstanceObject*>(objVal)) {
                    InstanceObject* instance = std::get<InstanceObject*>(objVal);
                    ClassObject* contextClass = frame.function->isMethod ? frame.function->ownerClass : nullptr;
                    
                    if (instance->fields.count(name) || instance->klass->fields.count(name)) {
                        AccessModifier access = AccessModifier::PUBLIC;
                        if (instance->klass->fields.count(name)) access = instance->klass->fields[name];
                        if (checkAccess(instance->klass, contextClass, access)) {
                            if (instance->fields.count(name)) {
                                push(instance->fields[name]);
                            } else {
                                push(std::monostate{}); 
                            }
                            break;
                        }
                    } else if (instance->klass->methods.count(name)) {
                        AccessModifier access = instance->klass->methodAccess[name];
                        if (checkAccess(instance->klass, contextClass, access)) {
                            std::vector<FunctionObject*> mVec = instance->klass->methods[name];
                            BoundMethod* bound = new BoundMethod(instance, mVec);
                            push(bound);
                            break;
                        }
                    }
                }
                
                push(globals[name]);
                break;
            }

            case OP_SET_PROPERTY_OR_LOCAL: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                Value objVal = pop(); 
                Value value = pop();
                
                if (std::holds_alternative<InstanceObject*>(objVal)) {
                    InstanceObject* instance = std::get<InstanceObject*>(objVal);
                    ClassObject* contextClass = frame.function->isMethod ? frame.function->ownerClass : nullptr;
                    
                    bool allowed = true;
                    if (instance->klass->fields.count(name)) {
                        AccessModifier access = instance->klass->fields[name];
                        allowed = checkAccess(instance->klass, contextClass, access);
                    }
                    
                    if (allowed) {
                        instance->fields[name] = value;
                        push(value);
                        break;
                    }
                }
                
                globals[name] = value;
                push(value);
                break;
            }
            
            case OP_INHERIT: {
                Value superclassVal = pop();
                if (!std::holds_alternative<ClassObject*>(superclassVal)) {
                    std::cerr << "Runtime error: Superclass must be a class." << std::endl;
                    return;
                }
                ClassObject* superclass = std::get<ClassObject*>(superclassVal);
                
                Value subclassVal = stack.back();
                ClassObject* subclass = std::get<ClassObject*>(subclassVal);
                
                subclass->parent = superclass;
                
                for (const auto& methodPair : superclass->methods) {
                    if (superclass->methodAccess[methodPair.first] == AccessModifier::PRIVATE) continue;
                    subclass->methods[methodPair.first] = methodPair.second;
                    subclass->methodAccess[methodPair.first] = superclass->methodAccess[methodPair.first];
                }
                for (const auto& fieldPair : superclass->fields) {
                    if (fieldPair.second == AccessModifier::PRIVATE) continue;
                    subclass->fields[fieldPair.first] = fieldPair.second;
                }
                break;
            }
            
            case OP_FIELD: {
                uint8_t nameIdx = frame.function->chunk.code[frame.ip++];
                std::string name = std::get<std::string>(frame.function->chunk.constants[nameIdx]);
                uint8_t modifier = frame.function->chunk.code[frame.ip++];
                
                Value klassVal = stack.back();
                ClassObject* klass = std::get<ClassObject*>(klassVal);
                klass->fields[name] = static_cast<AccessModifier>(modifier);
                break;
            }
            case OP_CAST_INT: {
                Value v = pop();

                if(std::holds_alternative<std::string>(v))
                    push(Value((int64_t)std::stoll(std::get<std::string>(v))));
                else if(std::holds_alternative<double>(v))
                    push(Value((int64_t)std::get<double>(v)));
                else if(std::holds_alternative<bool>(v))
                    push(Value((int64_t)std::get<bool>(v)));
                else if(std::holds_alternative<char>(v))
                    push(Value((int64_t)std::get<char>(v)));
                else
                    push(v); // No-op for already int or unsupported

                break;
            }
            case OP_CAST_FLOAT: {
                Value v = pop();

                if(std::holds_alternative<std::string>(v))
                    push(Value((double)std::stod(std::get<std::string>(v))));
                else if(std::holds_alternative<int64_t>(v))
                    push(Value((double)std::get<int64_t>(v)));
                else if(std::holds_alternative<bool>(v))
                    push(Value((double)std::get<bool>(v)));
                else if(std::holds_alternative<char>(v))
                    push(Value((double)std::get<char>(v)));
                else
                    push(v); // No-op for already double or unsupported

                break;
            }
            case OP_CAST_STRING: {
                Value v = pop();

                push(Value(valueToString(v)));

                break;
            }
            case OP_CAST_BOOL: {
                Value v = pop();

                if(std::holds_alternative<int64_t>(v))
                    push(Value((bool)std::get<int64_t>(v)));
                else if(std::holds_alternative<double>(v))
                    push(Value((bool)std::get<double>(v)));
                else if(std::holds_alternative<std::string>(v))
                    push(Value(!std::get<std::string>(v).empty()));
                else if(std::holds_alternative<char>(v))
                    push(Value((bool)std::get<char>(v)));
                else
                    push(v); // No-op for already bool or unsupported

                break;
            }
            case OP_CAST_CHAR: {
                Value v = pop();

                if(std::holds_alternative<int64_t>(v))
                    push(Value((char)std::get<int64_t>(v)));
                else if(std::holds_alternative<double>(v))
                    push(Value((char)std::get<double>(v)));
                else if(std::holds_alternative<std::string>(v)) {
                    std::string str = std::get<std::string>(v);
                    push(Value((char)(str.empty() ? 0 : str[0])));
                } else if(std::holds_alternative<bool>(v))
                    push(Value((char)std::get<bool>(v)));
                else
                    push(v); // No-op for already char or unsupported

                break;
            }

            case OP_HALT:
                return;
        }
    }
}

}
