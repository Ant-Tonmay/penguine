#include "interpreter/stmt_executor.h"
#include "interpreter/interpreter.h"
#include <iostream>

StmtExecutor::StmtExecutor(Interpreter* interpreter) : interpreter(interpreter) {}

void StmtExecutor::execute(const Stmt* stmt, Environment* env) {
    if (auto print = dynamic_cast<const PrintStmt*>(stmt)) {
        visit(print, env);
    } else if (auto assign = dynamic_cast<const AssignmentStmt*>(stmt)) {
        visit(assign, env);
    } else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        visit(ifStmt, env);
    } else if (auto forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        visit(forStmt, env);
    } else if (auto returnStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        visit(returnStmt, env);
    } else if (auto block = dynamic_cast<const Block*>(stmt)) {
        visit(block, env);
    } else if (auto block = dynamic_cast<const Block*>(stmt)) {
        visit(block, env);
    } else if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        visit(exprStmt, env);
    }
}

void StmtExecutor::visit(const ExprStmt* stmt, Environment* env) {
    interpreter->evaluateExpr(stmt->expression.get(), env);
}

void StmtExecutor::executeBlock(const Block* block, Environment* env) {
    
    Environment localEnv(env);
    for (const auto& stmt : block->statements) {
        execute(stmt.get(), &localEnv);
    }
}

void StmtExecutor::visit(const PrintStmt* stmt, Environment* env) {
    Value val = interpreter->evaluateExpr(stmt->expression.get(), env);
    printValue(val);
    std::cout << std::endl;
}

void StmtExecutor::visit(const AssignmentStmt* stmt, Environment* env) {
    for (const auto& assignment : stmt->assignments) {
        Value val = interpreter->evaluateExpr(assignment.value.get(), env);
        
        if (auto var = dynamic_cast<const VarExpr*>(assignment.target.get())) {
            try {
                env->assign(var->name, val);
            } catch (const std::runtime_error&) {
              
                env->define(var->name, val);
            }
        } else if (auto idx = dynamic_cast<const IndexExpr*>(assignment.target.get())) {
            Value arrVal = interpreter->evaluateExpr(idx->array.get(), env);
            Value idxVal = interpreter->evaluateExpr(idx->index.get(), env);
            
            if (std::holds_alternative<ArrayObject*>(arrVal) && std::holds_alternative<int>(idxVal)) {
                auto arr = std::get<ArrayObject*>(arrVal);
                int i = std::get<int>(idxVal);
                if (i >= 0 && (size_t)i < arr->length) {
                    arr->data[i] = val; // Deep copy? Primitive values are copy. Array/Obj are ptr.
                } else {
                    throw std::runtime_error("Index out of bounds");
                }
            } else {
                throw std::runtime_error("Invalid assignment target (index/array type mismatch)");
            }
        }
    }
}

void StmtExecutor::visit(const IfStmt* stmt, Environment* env) {
    Value cond = interpreter->evaluateExpr(stmt->condition.get(), env);
    bool isTrue = false;
    if (std::holds_alternative<bool>(cond)) isTrue = std::get<bool>(cond);
    else if (std::holds_alternative<int>(cond)) isTrue = std::get<int>(cond) != 0;
    
    if (isTrue) {
        executeBlock(stmt->thenBranch.get(), env);
    } else if (stmt->elseBranch) {
        if (auto block = dynamic_cast<const Block*>(stmt->elseBranch.get())) {
             executeBlock(block, env);
        } else {
             execute(stmt->elseBranch.get(), env);
        }
    }
}

void StmtExecutor::visit(const ForStmt* stmt, Environment* env) {
    Environment loopEnv(env); // Loop scope
    
    if (stmt->init) execute(stmt->init.get(), &loopEnv);
    
    while (true) {
        if (stmt->condition) {
            Value cond = interpreter->evaluateExpr(stmt->condition.get(), &loopEnv);
            bool isTrue = false;
            if (std::holds_alternative<bool>(cond)) isTrue = std::get<bool>(cond);
            else if (std::holds_alternative<int>(cond)) isTrue = std::get<int>(cond) != 0;
            if (!isTrue) break;
        }
        
        executeBlock(stmt->body.get(), &loopEnv);
        
        if (stmt->increment) execute(stmt->increment.get(), &loopEnv);
    }
}

void StmtExecutor::visit(const ReturnStmt* stmt, Environment* env) {
    Value val = std::monostate{};
    if (stmt->value) {
        val = interpreter->evaluateExpr(stmt->value.get(), env);
    }
    throw ReturnException{val};
}

void StmtExecutor::visit(const Block* stmt, Environment* env) {
    executeBlock(stmt, env);
}


