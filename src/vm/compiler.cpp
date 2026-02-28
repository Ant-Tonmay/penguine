#include "vm/compiler.h"
#include <string>

namespace vm {

void Compiler::emit(uint8_t byte){
    chunk.write(byte);
}

void Compiler::emitConstant(Value v){
    int idx = chunk.addConstant(v);
    emit(OP_CONSTANT);
    emit(idx);
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
        for (const auto& stmt : block->statements) {
            compileStmt(stmt.get());
        }
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
    else if (auto* bin = dynamic_cast<BinaryExpr*>(node)) {
        compileExpr(bin->left.get());
        compileExpr(bin->right.get());

        if (bin->op == "+") emit(OP_ADD);
        else if (bin->op == "-") emit(OP_SUB);
        else if (bin->op == "*") emit(OP_MUL);
        else if (bin->op == "/") emit(OP_DIV);
    }
}

void Compiler::compileStmt(ASTNode* node) {
    if (auto* printStmt = dynamic_cast<PrintStmt*>(node)) {
        compileExpr(printStmt->expression.get());
        emit(OP_PRINT);
    }
    else if (auto* exprStmt = dynamic_cast<ExprStmt*>(node)) {
        compileExpr(exprStmt->expression.get());
    }
    else if (dynamic_cast<Expr*>(node)) {
        compileExpr(node);
    }
}

}
