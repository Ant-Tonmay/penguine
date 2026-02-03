#include "interpreter/interpreter.h"
#include <iostream>
#include <stdexcept>

Interpreter::Interpreter() : globals(new SymbolTable()), current(globals) {}

void Interpreter::executeProgram(const Program* program) {
    // Find the "main" function
    const Function* mainFunction = nullptr;
    for (const auto& func : program->functions) {
        if (func->name == "main") {
            mainFunction = func.get();
            break;
        }
    }

    if (mainFunction) {
        executeBlock(mainFunction->body.get());
    } else {
        std::cerr << "Error: No 'main' function found." << std::endl;
    }
}

void Interpreter::executeBlock(const Block* block) {
    SymbolTable* previous = current;
    current = new SymbolTable(previous); 

    try {
        for (const auto& stmt : block->statements) {
            execute(stmt.get());
        }
    } catch (...) {
        delete current;
        current = previous;
        throw;
    }

    SymbolTable* temp = current;
    current = previous;
    delete temp;
}

void Interpreter::execute(const Stmt* stmt) {
    if (auto printStmt = dynamic_cast<const PrintStmt*>(stmt)) {
        Value value = evaluate(printStmt->expression.get());
        if (std::holds_alternative<int>(value)) {
            std::cout << std::get<int>(value) << std::endl;
        } else if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value) << std::endl;
        } else if (std::holds_alternative<double>(value)) {
            std::cout << std::get<double>(value) << std::endl;
        } else if (std::holds_alternative<float>(value)) {
            std::cout << std::get<float>(value) << std::endl;
        } else if (std::holds_alternative<long>(value)) {
            std::cout << std::get<long>(value) << std::endl;
        } else if (std::holds_alternative<long long>(value)) {
            std::cout << std::get<long long>(value) << std::endl;
        } else if (std::holds_alternative<bool>(value)) {
            std::cout << (std::get<bool>(value) ? "true" : "false") << std::endl;
        } else if (std::holds_alternative<char>(value)) {
            std::cout << std::get<char>(value) << std::endl;
        }
    } else if (auto declStmt = dynamic_cast<const AssignmentStmt*>(stmt)) {
        for (const auto& assignment : declStmt->assignments) {
            Value value = evaluate(assignment.value.get());
             
            try {
                 current->assign(assignment.name, value);
            } catch (const std::runtime_error&) {
                 current->define(assignment.name, value);
            }
        }
    } else if (auto forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        // Create a new scope for the loop
        SymbolTable* previous = current;
        current = new SymbolTable(previous);

        try {
            if (forStmt->init) execute(forStmt->init.get()); // Init
            
            while (true) {
                // Condition
                if (forStmt->condition) {
                     Value cond = evaluate(forStmt->condition.get());
                     if (std::holds_alternative<bool>(cond)) {
                         if (!std::get<bool>(cond)) break;
                     } else {
                         // Non-bool condition? Treat as true if not 0? 
                         // For simplicity, error if not bool
                         throw std::runtime_error("For loop condition must be boolean");
                     }
                }
                
                // Body
                executeBlock(forStmt->body.get());

                // Increment
                if (forStmt->increment) execute(forStmt->increment.get());
            }
        } catch (...) {
            delete current;
            current = previous;
            throw;
        }
        
        SymbolTable* temp = current;
        current = previous;
        delete temp;
    }
    else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {

        Value cond = evaluate(ifStmt->condition.get());

        bool conditionTrue = false;

        if (std::holds_alternative<bool>(cond)) {
            conditionTrue = std::get<bool>(cond);
        } else if (std::holds_alternative<int>(cond)) {
            conditionTrue = std::get<int>(cond) != 0;
        } else {
            throw std::runtime_error("If condition must be boolean or integer");
        }

        if (conditionTrue) {
            executeBlock(ifStmt->thenBranch.get());
        } else if (ifStmt->elseBranch) {
            execute(ifStmt->elseBranch.get());
        }
    } 
    
    else if (auto block = dynamic_cast<const Block*>(stmt)) {
        executeBlock(block);
    }
}


Value Interpreter::evaluate(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        // Simple parsing, assuming integer for now if no dot
        if (num->value.find('.') != std::string::npos) {
             return std::stod(num->value);
        }
        return std::stoi(num->value);
    } else if (auto str = dynamic_cast<const StringExpr*>(expr)) {
        return str->value;
    } else if (auto var = dynamic_cast<const VarExpr*>(expr)) {
        return current->get(var->name);
    } else if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        Value left = evaluate(bin->left.get());
        Value right = evaluate(bin->right.get());

        // Basic implementation for numbers (int only for simplicity start)
        // Need comprehensive visitor or helpers for proper type handling
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            int l = std::get<int>(left);
            int r = std::get<int>(right);
            if (bin->op == "+") return l + r;
            if (bin->op == "-") return l - r;
            if (bin->op == "*") return l * r;
            if (bin->op == "/") return l / r; 
            if (bin->op == "<") return l < r;
            if (bin->op == ">") return l > r;
            if (bin->op == "==") return l == r;
        }
        // Handle strings for +
        if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
            if (bin->op == "+") return std::get<std::string>(left) + std::get<std::string>(right);
             if (bin->op == "==") return std::get<std::string>(left) == std::get<std::string>(right);
        }
        
    }
    return std::monostate{};
}
