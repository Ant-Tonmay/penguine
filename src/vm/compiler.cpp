#include "vm/compiler.h"
#include <string>
#include "lexer/lexer.h"

namespace vm {

void Compiler::emit(uint8_t byte){
    chunk.write(byte);
}

void Compiler::emitConstant(Value v){
    int idx = chunk.addConstant(v);
    emit(OP_CONSTANT);
    emit(idx);  
}

void Compiler::beginScope() {
    scopeDepth++;
}

void Compiler::endScope() {
    scopeDepth--;
    while (locals.size() > 0 && locals.back().depth > scopeDepth) {
        emit(OP_POP);
        locals.pop_back();
    }
}

void Compiler::addLocal(const std::string& name) {
    locals.push_back({name, scopeDepth});
}

int Compiler::resolveLocal(const std::string& name) {
    for (int i = locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
            return i;
        }
    }
    return -1;
}

void Compiler::compile(ASTNode* node){
    if (auto* program = dynamic_cast<Program*>(node)) {
        bool foundMain = false;
        for (const auto& func : program->functions) {
            if (func->name == "main") {
                compile(func->body.get());
                emit(OP_HALT);
                foundMain = true;
                break;
            }
        }
        if (!foundMain) {
            emit(OP_HALT);
        }
    } 
    else if (auto* block = dynamic_cast<Block*>(node)) {
        beginScope();
        for (const auto& stmt : block->statements) {
            compileStmt(stmt.get());
        }
        endScope();
    }
    else {
        compileStmt(node);
        emit(OP_RETURN);
    }
}

void Compiler::compileExpr(ASTNode* node){
    if (auto* num = dynamic_cast<NumberExpr*>(node)) {
        try {
            double v = std::stod(num->value);
            emitConstant(v);
        } catch (...) {}
    }
    else if (auto* var = dynamic_cast<VarExpr*>(node)) {
        int arg = resolveLocal(var->name);
        if (arg != -1) {
            emit(OP_GET_LOCAL);
            emit(arg);
        } else {
            Value nameVal = var->name;
            int idx = chunk.addConstant(nameVal);
            emit(OP_GET_GLOBAL);
            emit(idx);
        }
    }
    else if (auto* bin = dynamic_cast<BinaryExpr*>(node)) {
        compileExpr(bin->left.get());
        compileExpr(bin->right.get());

        if (bin->op == "+") emit(OP_ADD);
        else if (bin->op == "-") emit(OP_SUB);
        else if (bin->op == "*") emit(OP_MUL);
        else if (bin->op == "/") emit(OP_DIV);
        else if (bin->op == ">") emit(OP_GREATER);
        else if (bin->op == ">=") emit(OP_GREATER_EQUAL);
        else if (bin->op == "<") emit(OP_LESSER);
        else if (bin->op == "<=") emit(OP_LESSER_EQUAL);
        else if (bin->op == ">>") emit(OP_RIGHT_SHIFT);
        else if (bin->op == "<<") emit(OP_LEFT_SHIFT);
        else if (bin->op == "|") emit(OP_BITWISE_OR);
        else if (bin->op == "&") emit(OP_BITWISE_AND);
        else if (bin->op == "||") emit(OP_LOGICAL_OR);
        else if (bin->op == "&&") emit(OP_LOGICAL_AND);
        else if (bin->op == "==") emit(OP_EQUAL);
        else if (bin->op == "!=") emit(OP_NOT_EQUAL);
        else if (bin->op == "^") emit(OP_XOR);
        else if (bin->op == "+=") emit(OP_PLUS_EQUAL);
        else if (bin->op == "-=") emit(OP_MINUS_EQUAL);
        else if (bin->op == "*=") emit(OP_MULTIPLY_EQUAL);
        else if (bin->op == "/=") emit(OP_DIVIDE_EQUAL);
        else if (bin->op == "%=") emit(OP_MODULO_EQUAL);
        else if (bin->op == "<<=") emit(OP_LEFT_SHIFT_EQUAL);
        else if (bin->op == ">>=") emit(OP_RIGHT_SHIFT_EQUAL);
        else if (bin->op == "&=") emit(OP_BITWISE_AND_EQUAL);
        else if (bin->op == "|=") emit(OP_BITWISE_OR_EQUAL);
        else if (bin->op == "^=") emit(OP_XOR_EQUAL);
        else if (bin->op == "&&=") emit(OP_LOGICAL_AND_EQUAL);
        else if (bin->op == "||") emit(OP_LOGICAL_OR_EQUAL);  
    }
}

void Compiler::compileStmt(ASTNode* node) {
    if (auto* printStmt = dynamic_cast<PrintStmt*>(node)) {
        compileExpr(printStmt->expression.get());
        emit(OP_PRINT);
    }
    else if (auto* exprStmt = dynamic_cast<ExprStmt*>(node)) {
        compileExpr(exprStmt->expression.get());
        emit(OP_POP);
    }
    else if (auto* assignStmt = dynamic_cast<AssignmentStmt*>(node)) {
        for (const auto& assign : assignStmt->assignments) {
            if (auto* var = dynamic_cast<VarExpr*>(assign.target.get())) {
                int arg = resolveLocal(var->name);
                bool isLocal = true;
                bool isNewLocal = false;

                if (arg == -1 && scopeDepth > 0) {
                     // Inside a function, any assignment to an unknown variable becomes a new local!
                     addLocal(var->name);
                     arg = locals.size() - 1;
                     isNewLocal = true;
                } else if (arg == -1 && scopeDepth == 0) {
                     // Global scope
                     isLocal = false;
                     Value nameVal = var->name;
                     arg = chunk.addConstant(nameVal);
                }

                if (assign.op != TokenType::EQUAL) {
                    if (isLocal) {
                        emit(OP_GET_LOCAL);
                        emit(arg);
                    } else {
                        emit(OP_GET_GLOBAL);
                        emit(arg);
                    }
                    compileExpr(assign.value.get());
                    
                    switch(assign.op) {
                        case TokenType::PLUS_EQUAL: emit(OP_PLUS_EQUAL); break;
                        case TokenType::MINUS_EQUAL: emit(OP_MINUS_EQUAL); break;
                        case TokenType::STAR_EQUAL: emit(OP_MULTIPLY_EQUAL); break;
                        case TokenType::SLASH_EQUAL: emit(OP_DIVIDE_EQUAL); break;
                        case TokenType::MOD_OP_EQUAL: emit(OP_MODULO_EQUAL); break;
                        case TokenType::BITWISE_AND_EQUAL: emit(OP_BITWISE_AND_EQUAL); break;
                        case TokenType::BITWISE_OR_EQUAL: emit(OP_BITWISE_OR_EQUAL); break;
                        case TokenType::XOR_EQUAL: emit(OP_XOR_EQUAL); break;
                        default: break; // Or OP_ADD
                    }
                } else {
                    compileExpr(assign.value.get());
                }
                
                if (isNewLocal) {
                    // Eat Five star , Do Nothing for now
                } else if (isLocal) {
                    emit(OP_SET_LOCAL);
                    emit(arg);
                    emit(OP_POP);
                } else {
                    emit(OP_SET_GLOBAL);
                    emit(arg);
                    emit(OP_POP);
                }
            }
        }
    }
    else if (auto* block = dynamic_cast<Block*>(node)) {
        beginScope();
        for (const auto& stmt : block->statements) {
            compileStmt(stmt.get());
        }
        endScope();
    }
    else if (dynamic_cast<Expr*>(node)) {
        compileExpr(node);
        emit(OP_POP);
    }
}

}
