#pragma once

#include "lexer/lexer.h"
#include "parser/ast.h"
#include <vector>
#include <memory>
#include <string>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();
    std::unique_ptr<Expr> parseExpression();

private:
    const std::vector<Token>& tokens;
    size_t current;
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;

    std::unique_ptr<Function> parseFunction();
    std::unique_ptr<Block> parseBlock();
    
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<PrintStmt> parsePrintStmt();
    std::unique_ptr<AssignmentStmt> parseAssignmentStmt();
    std::unique_ptr<ForStmt> parseForStmt();
    std::unique_ptr<IfStmt> parseIfStmt();

    std::unique_ptr<Expr> expression(); 
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseShift();
    std::unique_ptr<Expr> parseBitwiseOr();
    std::unique_ptr<Expr> parseBitwiseAnd();
    std::unique_ptr<Expr> parseBitwiseXor();
};
