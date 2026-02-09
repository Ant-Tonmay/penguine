#include "interpreter/interpreter.h"
#include <iostream>
#include <stdexcept>
#include <interpreter/control_flow.h>
#include "interpreter/runtime_value.h"

Interpreter::Interpreter() {
    globals = new Environment();
    evaluator = std::make_unique<ExprEvaluator>(this);
    executor = std::make_unique<StmtExecutor>(this);
}

Interpreter::~Interpreter() {
    delete globals;
}

void Interpreter::executeProgram(const Program* program) {
   
    for (const auto& func : program->functions) {
        userFunctions[func->name] = func.get();
    }
    
    if (userFunctions.count("main")) {
        callUserFunction(userFunctions["main"], {});
    } else {
        std::cerr << "Error: No 'main' function found." << std::endl;
    }
}

Value Interpreter::evaluateExpr(const Expr* expr, Environment* env) {
    return evaluator->evaluate(expr, env);
}

void Interpreter::executeStmt(const Stmt* stmt, Environment* env) {
    executor->execute(stmt, env);
}

void Interpreter::executeBlock(const Block* block, Environment* env) {
    executor->executeBlock(block, env);
}

Value Interpreter::callFunctionByName(const std::string& name, const std::vector<Value>& args) {

    if (name == "print") {
        for (const auto& arg : args) {
            printValue(arg);
        }
        std::cout << std::endl;
        return std::monostate{};
    }
    else if (name == "fixed") {
        // fixed(size, init?) -> ArrayObject*
        if (args.size() < 1 || args.size() > 2) throw std::runtime_error("fixed(size, init?) expects 1 or 2 arguments.");
        if (!std::holds_alternative<int>(args[0])) throw std::runtime_error("fixed() size must be an integer.");
        
        int size = std::get<int>(args[0]);
        if (size < 0) throw std::runtime_error("fixed() size cannot be negative.");
        
        Value initVal = std::monostate{};
        if (args.size() == 2) {
            initVal = args[1];
            // Auto-unwrap if it's an array of size 1
            if (std::holds_alternative<ArrayObject*>(initVal)) {
                ArrayObject* initArr = std::get<ArrayObject*>(initVal);
                if (initArr->length == 1) {
                    initVal = initArr->data[0];
                }
            }
        }
        
        ArrayObject* arr = new ArrayObject();
        arr->isFixed = true;
        arr->length = size;
        arr->capacity = size;
        arr->data = new Value[size];
        
        for(int i=0; i<size; ++i) {
            arr->data[i] = deepCopyIfNeeded(initVal);
        }
        
        return arr;
    }
    else if (name == "push") {
         
         if (args.size() != 2) throw std::runtime_error("push(array, value) expects 2 arguments.");
         Value arrVal = args[0];
         Value elem = args[1];
         
         if (!std::holds_alternative<ArrayObject*>(arrVal)) throw std::runtime_error("First argument to push must be an array.");
         ArrayObject* arr = std::get<ArrayObject*>(arrVal);
         
         if (arr->isFixed) throw std::runtime_error("Cannot push to fixed array.");
         
         // Resize logic
         if (arr->length == arr->capacity) {
             size_t newCap = arr->capacity == 0 ? 4 : arr->capacity * 2;
             Value* newData = new Value[newCap];
             for(size_t i=0; i<arr->length; ++i) newData[i] = arr->data[i];
             delete[] arr->data;
             arr->data = newData;
             arr->capacity = newCap;
         }
         arr->data[arr->length++] = deepCopyIfNeeded(elem);
         
         return std::monostate{};
    }
    
    // User defined
    if (userFunctions.count(name)) {
        return callUserFunction(userFunctions[name], args);
    }
    
    throw std::runtime_error("Undefined function: " + name);
}

Value Interpreter::callUserFunction(Function* fn, const std::vector<Value>& args) {
    if (args.size() != fn->params.size()) {
        throw std::runtime_error("Function " + fn->name + " expects " + std::to_string(fn->params.size()) + " arguments.");
    }

    Environment* fnEnv = new Environment(globals);
    for (size_t i = 0; i < fn->params.size(); ++i) {
        if (fn->params[i].isRef) {
          
            
            fnEnv->define(fn->params[i].name, args[i]);
        } else {
            fnEnv->define(fn->params[i].name, deepCopyIfNeeded(args[i]));
        }
    }
    
    Value retVal = std::monostate{};
    try {
        executor->executeBlock(fn->body.get(), fnEnv);
    } catch (const ReturnException& e) {
        retVal = e.value;
    }
    
    delete fnEnv;
    return retVal;
}

Value Interpreter::deepCopyIfNeeded(const Value& v) {
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
