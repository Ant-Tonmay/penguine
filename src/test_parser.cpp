#include "../include/parser/parser.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

// Helper to assert that an expression evaluates to a specific structure or string representation
// For now, since we only have AST structure, we'll implement a simple toString on the Expr classes 
// just for testing purposes or rely on dynamic_cast checks. 
// A better way for these tests is to perhaps add a virtual toString() or visit() method to AST.
// But since I can't easily modify AST without recompiling everything and maybe user didn't ask for that yet,
// let's try to verify by dynamic casting.

void testNumber() {
    // "123"
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "123"),
        Token(TokenType::EOF_TOKEN, "")
    };
    Parser parser(tokens);
    auto expr = parser.parse();

    auto* num = dynamic_cast<NumberExpr*>(expr.get());
    assert(num != nullptr);
    assert(num->value == "123");
    
    std::cout << "testNumber passed" << std::endl;
}

void testBinaryOp() {
    // "1 + 2"
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::EOF_TOKEN, "")
    };
    Parser parser(tokens);
    auto expr = parser.parse();

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    assert(bin != nullptr);
    assert(bin->op == "+");
    
    auto* left = dynamic_cast<NumberExpr*>(bin->left.get());
    assert(left != nullptr && left->value == "1");

    auto* right = dynamic_cast<NumberExpr*>(bin->right.get());
    assert(right != nullptr && right->value == "2");

    std::cout << "testBinaryOp passed" << std::endl;
}

void testPrecedence() {
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
    auto expr = parser.parse();

    // Should be BinaryExpr(+) with left=1 and right=BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    assert(root != nullptr);
    assert(root->op == "+");

    auto* left = dynamic_cast<NumberExpr*>(root->left.get());
    assert(left != nullptr && left->value == "1");

    auto* rightBin = dynamic_cast<BinaryExpr*>(root->right.get());
    assert(rightBin != nullptr);
    assert(rightBin->op == "*");
    
    auto* rLeft = dynamic_cast<NumberExpr*>(rightBin->left.get());
    assert(rLeft != nullptr && rLeft->value == "2");

    auto* rRight = dynamic_cast<NumberExpr*>(rightBin->right.get());
    assert(rRight != nullptr && rRight->value == "3");
    
    std::cout << "testPrecedence passed" << std::endl;
}

void testGrouping() {
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
    auto expr = parser.parse();

    // Should be BinaryExpr(*) with left=BinaryExpr(+) and right=3
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    assert(root != nullptr);
    assert(root->op == "*");

    auto* leftBin = dynamic_cast<BinaryExpr*>(root->left.get());
    assert(leftBin != nullptr);
    assert(leftBin->op == "+");

    auto* right = dynamic_cast<NumberExpr*>(root->right.get());
    assert(right != nullptr && right->value == "3");

    std::cout << "testGrouping passed" << std::endl;
}

void testExtraTokens() {
    // "1 2" -> Should parse "1" and then fail because "2" remains
    std::vector<Token> tokens = {
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::EOF_TOKEN, "")
    };

    Parser parser(tokens);
    try {
        parser.parse();
        std::cerr << "testExtraTokens failed: Expected exception for extra tokens" << std::endl;
        exit(1);
    } catch (const std::exception& e) {
        std::cout << "testExtraTokens passed" << std::endl;
    }
}

int main() {
    std::cout << "Running Parser Tests..." << std::endl;
    testNumber();
    testBinaryOp();
    testPrecedence();
    testGrouping();
    testExtraTokens();
    std::cout << "All parser tests passed!" << std::endl;
    return 0;
}
