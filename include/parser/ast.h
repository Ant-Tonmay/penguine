#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// --------------------
// Base AST Node
// --------------------
struct ASTNode {
    virtual ~ASTNode() = default;
};

// --------------------
// Expressions
// --------------------
struct Expr : ASTNode {};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct NumberExpr : Expr {
    std::string value;
    explicit NumberExpr(std::string value) : value(value) {}
};

struct StringExpr : Expr {
    std::string value;
    explicit StringExpr(std::string value) : value(value) {}
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string name) : name(name) {}
};

// --------------------
// Statements
// --------------------
struct Stmt : ASTNode {};

struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expression;
    explicit PrintStmt(std::unique_ptr<Expr> expression) 
        : expression(std::move(expression)) {}
};

struct Assignment {
    std::string name;
    std::unique_ptr<Expr> value;
    
    Assignment(std::string name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
};

struct AssignmentStmt : Stmt {
    std::vector<Assignment> assignments;
    
    explicit AssignmentStmt(std::vector<Assignment> assignments)
        : assignments(std::move(assignments)) {}
};

struct ForStmt : Stmt {
    std::unique_ptr<AssignmentStmt> init;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<AssignmentStmt> increment; // Parsing as assignment for now per grammar
    std::unique_ptr<Block> body;

    ForStmt(std::unique_ptr<AssignmentStmt> init,
            std::unique_ptr<Expr> condition,
            std::unique_ptr<AssignmentStmt> increment,
            std::unique_ptr<Block> body)
        : init(std::move(init)), 
          condition(std::move(condition)), 
          increment(std::move(increment)), 
          body(std::move(body)) {}
};

struct Function : ASTNode {
    std::string name;
    std::unique_ptr<Block> body;

    Function(std::string name, std::unique_ptr<Block> body)
        : name(name), body(std::move(body)) {}
};

struct Program : ASTNode {
    std::vector<std::unique_ptr<Function>> functions;
};
