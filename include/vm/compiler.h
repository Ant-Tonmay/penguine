#pragma once
#include "chunk.h"
#include "../parser/ast.h"

namespace vm {

struct Local {
    std::string name;
    int depth;
};

class Compiler {
public:
    Chunk chunk;
    std::vector<Local> locals;
    int scopeDepth = 0;

    void compile(ASTNode* node);

private:
    void emit(uint8_t byte);
    void emitConstant(Value v);

    int emitJump(uint8_t instruction);
    void patchJump(int offset);
    void emitLoop(int loopStart);

    void beginScope();
    void endScope();
    void addLocal(const std::string& name);
    int resolveLocal(const std::string& name);

    void compileExpr(ASTNode*);
    void compileStmt(ASTNode*);
};

}
