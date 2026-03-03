#include "vm/compiler.h"
#include <string>
#include "lexer/lexer.h"

namespace vm {

// --- Jump Helpers ---
int Compiler::emitJump(uint8_t instruction) {
    emit(instruction);
    emit(0xff);
    emit(0xff);
    return chunk.code.size() - 2;
}

void Compiler::patchJump(int offset) {
    int jump = chunk.code.size() - offset - 2;
    if (jump > UINT16_MAX) {
        // Technically this should throw a compiler error
    }
    chunk.code[offset] = (jump >> 8) & 0xff;
    chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emit(OP_LOOP);
    int offset = chunk.code.size() - loopStart + 2;
    if (offset > UINT16_MAX) {
        // Technically this should throw a compiler error
    }
    emit((offset >> 8) & 0xff);
    emit(offset & 0xff);
}
// --------------------

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
    else if (auto* b = dynamic_cast<BoolExpr*>(node)) {
        if (b->value) emit(OP_TRUE);
        else emit(OP_FALSE);
    }
    else if (auto* s = dynamic_cast<StringExpr*>(node)) {
        emitConstant(s->value);
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

        // Short-circuiting for && and ||
        if (bin->op == "&&") {
            compileExpr(bin->left.get());
            int endJump = emitJump(OP_JUMP_IF_FALSE);
            emit(OP_POP); // Pop the left operand if it's true, because we need the right operand's value
            compileExpr(bin->right.get());
            patchJump(endJump);
            return;
        } 
        else if (bin->op == "||") {
            compileExpr(bin->left.get());
            // If left is true, jump to the end and keep 'true' (on the stack).
            // If false, jump to evaluate right. 
            int elseJump = emitJump(OP_JUMP_IF_FALSE);
            int endJump = emitJump(OP_JUMP);
            
            patchJump(elseJump);
            emit(OP_POP); // Left is false, pop it
            compileExpr(bin->right.get());
            
            patchJump(endJump);
            return;
        }

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
    else if (auto* ifStmt = dynamic_cast<IfStmt*>(node)) {
        compileExpr(ifStmt->condition.get());
        int thenJump = emitJump(OP_JUMP_IF_FALSE);
        emit(OP_POP); 
        
        compileStmt(ifStmt->thenBranch.get());
        
        int elseJump = emitJump(OP_JUMP);
        patchJump(thenJump);
        emit(OP_POP); // Pop condition when condition was false!
        
        if (ifStmt->elseBranch) {
            compileStmt(ifStmt->elseBranch.get());
        }
        
        patchJump(elseJump);
    }
    else if (auto* whileStmt = dynamic_cast<WhileStmt*>(node)) {
        int loopStart = chunk.code.size();
        
        compileExpr(whileStmt->condition.get());
        int exitJump = emitJump(OP_JUMP_IF_FALSE);
        emit(OP_POP); // Pop condition if we enter the loop
        
        compileStmt(whileStmt->body.get());
        emitLoop(loopStart);
        
        patchJump(exitJump);
        emit(OP_POP); // Pop condition if we just exited the loop
    }
    else if (auto* forStmt = dynamic_cast<ForStmt*>(node)) {
        beginScope();
        
        if (forStmt->init) {
            compileStmt(forStmt->init.get());
        }
        
        int loopStart = chunk.code.size();
        int exitJump = -1;
        
        if (forStmt->condition) {
            compileExpr(forStmt->condition.get());
            exitJump = emitJump(OP_JUMP_IF_FALSE);
            emit(OP_POP); // Pop condition if we enter the loop
        }
        
        compileStmt(forStmt->body.get());
        
        if (forStmt->increment) {
            compileStmt(forStmt->increment.get());
        }
        
        emitLoop(loopStart);
        
        if (exitJump != -1) {
            patchJump(exitJump);
            emit(OP_POP); // Pop condition if we just exited the loop
        }
        
        endScope();
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
