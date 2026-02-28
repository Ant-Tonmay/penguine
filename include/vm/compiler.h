#pragma once
#include "chunk.h"
#include "../parser/ast.h"

namespace vm {

class Compiler {
public:
    Chunk chunk;

    void compile(ASTNode* node);

private:
    void emit(uint8_t byte);
    void emitConstant(Value v);

    void compileExpr(ASTNode*);
    void compileStmt(ASTNode*);
};

}
