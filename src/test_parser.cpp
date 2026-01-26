#include "parser/parser.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

// Helper macros for verification
#define ASSERT_NOT_NULL(ptr) if (ptr == nullptr) { std::cerr << "Assertion failed: " #ptr " is null at line " << __LINE__ << std::endl; exit(1); }
#define ASSERT_EQ(val1, val2) if (val1 != val2) { std::cerr << "Assertion failed: " #val1 " (" << val1 << ") != " #val2 " (" << val2 << ") at line " << __LINE__ << std::endl; exit(1); }

void testNumber() {
    std::cout << "Testing Number..." << std::endl;
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "123"),
        Token(TokenType::EOF_TOKEN, "")
    };
    Parser parser(tokens);
    auto expr = parser.parseExpression(); // Changed to parseExpression

    auto* num = dynamic_cast<NumberExpr*>(expr.get());
    ASSERT_NOT_NULL(num);
    ASSERT_EQ(num->value, "123");
}

void testBinaryOp() {
    std::cout << "Testing BinaryOp..." << std::endl;
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::EOF_TOKEN, "")
    };
    Parser parser(tokens);
    auto expr = parser.parseExpression();

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    ASSERT_NOT_NULL(bin);
    ASSERT_EQ(bin->op, "+");
    
    auto* left = dynamic_cast<NumberExpr*>(bin->left.get());
    ASSERT_NOT_NULL(left);
    ASSERT_EQ(left->value, "1");

    auto* right = dynamic_cast<NumberExpr*>(bin->right.get());
    ASSERT_NOT_NULL(right);
    ASSERT_EQ(right->value, "2");
}

void testPrecedence() {
    std::cout << "Testing Precedence..." << std::endl;
    // "1 + 2 * 3" -> "1 + (2 * 3)"
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::STAR, "*"),
        Token(TokenType::NUMBER, "3"),
        Token(TokenType::EOF_TOKEN, "")
    };
    Parser parser(tokens);
    auto expr = parser.parseExpression();

    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(root->op, "+");

    auto* left = dynamic_cast<NumberExpr*>(root->left.get());
    ASSERT_NOT_NULL(left);
    ASSERT_EQ(left->value, "1");

    auto* rightBin = dynamic_cast<BinaryExpr*>(root->right.get());
    ASSERT_NOT_NULL(rightBin);
    ASSERT_EQ(rightBin->op, "*");
}

void testGrouping() {
    std::cout << "Testing Grouping..." << std::endl;
    // "(1 + 2) * 3"
    std::vector<Token> tokens = {
        Token(TokenType::LPAREN, "("),
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::STAR, "*"),
        Token(TokenType::NUMBER, "3"),
        Token(TokenType::EOF_TOKEN, "")
    };

    Parser parser(tokens);
    auto expr = parser.parseExpression();

    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(root->op, "*");

    auto* leftBin = dynamic_cast<BinaryExpr*>(root->left.get());
    ASSERT_NOT_NULL(leftBin);
    ASSERT_EQ(leftBin->op, "+");
}

void testAssignmentStmt() {
    std::cout << "Testing AssignmentStmt..." << std::endl;
    // a = 10, b = 20;
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "a"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::NUMBER, "10"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::IDENTIFIER, "b"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::NUMBER, "20"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::EOF_TOKEN, "")
    };
    
    // We need to wrap this in a block or handle it via parseBlock since parseStatement is private?
    // Actually parse() -> Program -> Function -> Block -> Statement
    // But we can check if we can expose parseStatement or just wrap it in a mock?
    // Or just test Program structure.
    
    // Let's create a full program structure for statement testing to match public API
    // { main() { a=10, b=20; } }
    
    std::vector<Token> progTokens = {
        Token(TokenType::LBRACE, "{"),
        Token(TokenType::IDENTIFIER, "main"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::LBRACE, "{"),
        
        Token(TokenType::IDENTIFIER, "a"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::NUMBER, "10"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::IDENTIFIER, "b"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::NUMBER, "20"),
        Token(TokenType::SEMICOLON, ";"),
        
        Token(TokenType::RBRACE, "}"),
        Token(TokenType::RBRACE, "}"),
        Token(TokenType::EOF_TOKEN, "")
    };
    
    Parser parser(progTokens);
    auto program = parser.parse();
    ASSERT_NOT_NULL(program);
    ASSERT_EQ(program->functions.size(), 1);
    
    auto& func = program->functions[0];
    ASSERT_EQ(func->name, "main");
    ASSERT_EQ(func->body->statements.size(), 1);
    
    auto* stmt = dynamic_cast<AssignmentStmt*>(func->body->statements[0].get());
    ASSERT_NOT_NULL(stmt);
    ASSERT_EQ(stmt->assignments.size(), 2);
    ASSERT_EQ(stmt->assignments[0].name, "a");
    ASSERT_EQ(stmt->assignments[1].name, "b");
}

int main() {
    std::cout << "Running Parser Tests..." << std::endl;
    testNumber();
    testBinaryOp();
    testPrecedence();
    testGrouping();
    testAssignmentStmt();
    std::cout << "All parser tests passed!" << std::endl;
    return 0;
}
