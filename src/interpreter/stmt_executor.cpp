#include "interpreter/stmt_executor.h"
#include "interpreter/interpreter.h"
#include "interpreter/control_flow.h"
#include <iostream>
#include "interpreter/runtime_value.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

StmtExecutor::StmtExecutor(Interpreter* interpreter) : interpreter(interpreter) {}

void StmtExecutor::execute(const Stmt* stmt, Environment* env) {
    if (auto print = dynamic_cast<const PrintStmt*>(stmt)) {
        visit(print, env);
    } else if (auto println = dynamic_cast<const PrintlnStmt*>(stmt)) {
        visit(println, env);
    } 
    else if (auto assign = dynamic_cast<const AssignmentStmt*>(stmt)) {
        visit(assign, env);
    } else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        visit(ifStmt, env);
    } else if (auto forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        visit(forStmt, env);
    } else if(auto whileStmt = dynamic_cast<const WhileStmt*>(stmt)){
        visit(whileStmt,env);
    } else if(auto breakStmt = dynamic_cast<const BreakStmt*>(stmt)){
        visit(breakStmt,env);
    }else if(auto continueStmt = dynamic_cast<const ContinueStmt*>(stmt)){
        visit(continueStmt,env);
    }else if (auto returnStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        visit(returnStmt, env);
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



std::string formatString(const std::string& str, Environment* env, Interpreter* interpreter) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '{') {
            size_t j = i + 1;
            while (j < str.length() && str[j] != '}') {
                j++;
            }
            
            if (j < str.length()) { 
                std::string content = str.substr(i + 1, j - i - 1);
                
                try {
                    Lexer lexer(content);
                    std::vector<Token> tokens = lexer.tokenize();
                    Parser parser(tokens);
                    auto expr = parser.parseExpression(); // We assume it's an expression
                    Value val = interpreter->evaluateExpr(expr.get(), env);
                    result += valueToString(val);
                } catch (const std::exception& e) {
                     // If parsing or evaluation fails, we can either throw or print the original text
                     // For now, let's treat it as a runtime error if user intended interpolation
                     throw std::runtime_error("Error interpolating string '" + content + "': " + e.what());
                }

                i = j; 
                continue;
            }
        }
        result += str[i];
    }
    return result;
}

void StmtExecutor::visit(const PrintStmt* stmt, Environment* env) {
    Value val = interpreter->evaluateExpr(stmt->expression.get(), env);
    if (std::holds_alternative<std::string>(val)) {
        std::string formatted = formatString(std::get<std::string>(val), env, interpreter);
        std::cout << formatted;
    } else {
        printValue(val);
    }
}

void StmtExecutor::visit(const PrintlnStmt* stmt, Environment* env) {
    Value val = interpreter->evaluateExpr(stmt->expression.get(), env);
    if (std::holds_alternative<std::string>(val)) {
        std::string formatted = formatString(std::get<std::string>(val), env, interpreter);
        std::cout << formatted;
    } else {
        printValue(val);
    }
    std::cout << std::endl;
}

Value performCompoundAssignment(TokenType op, const Value& currentVal, const Value& rightVal) {
    auto isInt = [](const Value& v) { return std::holds_alternative<int>(v); };
    auto isDouble = [](const Value& v) { return std::holds_alternative<double>(v); }; 
    
    if (std::holds_alternative<int>(currentVal) && std::holds_alternative<int>(rightVal)) {
        int l = std::get<int>(currentVal);
        int r = std::get<int>(rightVal);
        
        switch (op) {
            case TokenType::PLUS_EQUAL: return l + r;
            case TokenType::MINUS_EQUAL: return l - r;
            case TokenType::STAR_EQUAL: return l * r;
            case TokenType::SLASH_EQUAL: 
                if (r == 0) throw std::runtime_error("Division by zero in compound assignment.");
                return l / r; 
            case TokenType::MOD_OP_EQUAL:
                 if (r == 0) throw std::runtime_error("Modulo by zero in compound assignment.");
                 return l % r;
            case TokenType::BITWISE_AND_EQUAL: return l & r;
            case TokenType::BITWISE_OR_EQUAL: return l | r;
            case TokenType::XOR_EQUAL: return l ^ r;
            
            default: throw std::runtime_error("Unknown compound assignment operator.");
        }
    }
    
    throw std::runtime_error("Operands must be integers for compound assignment.");
}

void StmtExecutor::visit(const AssignmentStmt* stmt, Environment* env) {
    for (const auto& assignment : stmt->assignments) {
        Value val = interpreter->evaluateExpr(assignment.value.get(), env);
        
        if (auto var = dynamic_cast<const VarExpr*>(assignment.target.get())) {
            Value finalVal = val;

            if (assignment.op != TokenType::EQUAL) {
                
                Value currentVal = env->get(var->name);
                finalVal = performCompoundAssignment(assignment.op, currentVal, val);
            }
            try {
                env->assign(var->name, finalVal);
            } catch (const std::runtime_error&) {
                if (assignment.op == TokenType::EQUAL) {
                     env->define(var->name, finalVal);
                } else {
                     throw std::runtime_error("Undefined variable '" + var->name + "' for compound assignment.");
                }
            }
        } 
        else if (auto idx = dynamic_cast<const IndexExpr*>(assignment.target.get())) {
          
            Value arrVal = interpreter->evaluateExpr(idx->array.get(), env);
            Value idxVal = interpreter->evaluateExpr(idx->index.get(), env);
            if (std::holds_alternative<ArrayObject*>(arrVal) && std::holds_alternative<int>(idxVal)) {
                auto arr = std::get<ArrayObject*>(arrVal);
                int i = std::get<int>(idxVal);
                if (i < 0 || i >= arr->length) {
                    throw std::runtime_error("Array index out of bounds.");
                }
                Value finalVal = val;
                if (assignment.op != TokenType::EQUAL) {
                    Value currentVal = arrVal;
                    finalVal = performCompoundAssignment(assignment.op, currentVal, val);
                }
                arr->data[i] = finalVal;
            } else {
                throw std::runtime_error("Invalid array assignment target.");
            }
        } else {
             throw std::runtime_error("Invalid assignment target.");
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
bool StmtExecutor::isTruthy(const Value& v) {
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    if (std::holds_alternative<int>(v)) return std::get<int>(v) != 0;
    if (std::holds_alternative<std::monostate>(v)) return false;
    return true; // default truthy
}
void StmtExecutor::visit(const ForStmt* stmt, Environment* env) {
    Environment loopEnv(env); // Loop scope
    
    if (stmt->init) execute(stmt->init.get(), &loopEnv);
    
    while (true) {
        if (stmt->condition) {
            Value cond = interpreter->evaluateExpr(
                stmt->condition.get(), &loopEnv
            );
            if (!isTruthy(cond))
                break;
        }
        try {
            executeBlock(stmt->body.get(), &loopEnv);
        }
        catch (const ContinueSignal&) {
            //Do Nothing Eat Five Star
        }
        catch (const BreakSignal&) {
            break;
        }
        
        if (stmt->increment) execute(stmt->increment.get(), &loopEnv);
    }
}

void StmtExecutor::visit(const WhileStmt* stmt , Environment* env){

    Environment loopEnv(env); // Loop scope
    while (true) {
        Value cond = interpreter->evaluateExpr(stmt->condition.get(), &loopEnv);
        if (!isTruthy(cond))
            break;
        
        try {
            executeBlock(stmt->body.get(), &loopEnv);
        }
        catch (const ContinueSignal&) {
            //Do Nothing Eat Five Star
        }
        catch (const BreakSignal&) {
            break;
        }
        
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
void StmtExecutor::visit(const BreakStmt*, Environment*) {
    throw BreakSignal{};
}

void StmtExecutor::visit(const ContinueStmt*, Environment*) {
    throw ContinueSignal{};
}


