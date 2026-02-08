#pragma once

#include "parser/ast.h"
#include "interpreter/environment.h"
#include "interpreter/expr_evaluator.h"

class Interpreter;

class StmtExecutor {
public:
    StmtExecutor(Interpreter* interpreter);
    
    void execute(const Stmt* stmt, Environment* env);
    void executeBlock(const Block* block, Environment* env);

private:
    Interpreter* interpreter;

    void visit(const PrintStmt* stmt, Environment* env);
    void visit(const AssignmentStmt* stmt, Environment* env);
    void visit(const IfStmt* stmt, Environment* env);
    void visit(const ForStmt* stmt, Environment* env);
    void visit(const ReturnStmt* stmt, Environment* env);
    void visit(const Block* stmt, Environment* env); // Block is a stmt
    void visit(const ExprStmt* stmt, Environment* env);
};
