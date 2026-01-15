#pragma once

#include <memory>
#include <string>

// --------------------
// Base AST node
// --------------------
struct Expr {
    virtual ~Expr() = default;
};

// --------------------
// Literal number
// --------------------
struct NumberExpr : Expr {
    std::string value;   // keep as string for now

    explicit NumberExpr(const std::string& value)
        : value(value) {}
};

// --------------------
// Variable / identifier
// --------------------
struct IdentifierExpr : Expr {
    std::string name;

    explicit IdentifierExpr(const std::string& name)
        : name(name) {}
};

// --------------------
// Binary expression
// --------------------
struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left,
               const std::string& op,
               std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};
struct Stmt {
    virtual ~Stmt() = default;
};
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : expr(std::move(expr)) {}
};
struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBranch;
    std::unique_ptr<BlockStmt> elseBranch; // optional

    IfStmt(std::unique_ptr<Expr> cond,
           std::unique_ptr<BlockStmt> thenB,
           std::unique_ptr<BlockStmt> elseB)
        : condition(std::move(cond)),
          thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}
};
struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<BlockStmt> body;
};
struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::string callee,
             std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)),
          arguments(std::move(args)) {}
};
