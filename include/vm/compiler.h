#pragma once
#include "chunk.h"
#include "../parser/ast.h"
#include <vector>

namespace vm {

struct Local {
    std::string name;
    int depth;
};

struct LoopContext {
    int loopStart;                    // offset to jump back to for 'continue' (while loops)
    std::vector<int> breakJumps;      // pending forward jumps for 'break'
    std::vector<int> continueJumps;   // pending forward jumps for 'continue' (for loops)
};

class Compiler {
public:
    Chunk chunk;
    std::vector<Local> locals;
    int scopeDepth = 0;
    std::vector<LoopContext> loopStack;

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
