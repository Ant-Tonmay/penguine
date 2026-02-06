#include "parser/parser.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

// Helper macros for verification
#define ASSERT_NOT_NULL(ptr) if (ptr == nullptr) { std::cerr << "Assertion failed: " #ptr " is null at line " << __LINE__ << std::endl; exit(1); }
#define ASSERT_EQ(val1, val2) if (val1 != val2) { std::cerr << "Assertion failed: " #val1 " (" << val1 << ") != " #val2 " (" << val2 << ") at line " << __LINE__ << std::endl; exit(1); }

// Helper to wrap tokens in a main function for parsing
std::unique_ptr<Program> parseTokens(std::vector<Token>& tokens) {
    std::vector<Token> progTokens = {
        Token(TokenType::LBRACE, "{"),
        Token(TokenType::IDENTIFIER, "main"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::LBRACE, "{")
    };
    progTokens.insert(progTokens.end(), tokens.begin(), tokens.end());
    progTokens.push_back(Token(TokenType::RBRACE, "}"));
    progTokens.push_back(Token(TokenType::RBRACE, "}"));
    progTokens.push_back(Token(TokenType::EOF_TOKEN, ""));
    
    Parser parser(progTokens);
    return parser.parse();
}

void testBasicArray() {
    std::cout << "Testing Basic Array: [1, 2, 3]..." << std::endl;
    // arr = [1, 2, 3];
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "arr"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::LBRACKET, "["),
        Token(TokenType::NUMBER, "1"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::NUMBER, "2"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::NUMBER, "3"),
        Token(TokenType::RBRACKET, "]"),
        Token(TokenType::SEMICOLON, ";")
    };

    auto program = parseTokens(tokens);
    ASSERT_NOT_NULL(program);
    auto& func = program->functions[0];
    auto* assignStmt = dynamic_cast<AssignmentStmt*>(func->body->statements[0].get());
    ASSERT_NOT_NULL(assignStmt);
    
    auto* arrayExpr = dynamic_cast<ArrayExpr*>(assignStmt->assignments[0].value.get());
    ASSERT_NOT_NULL(arrayExpr);
    ASSERT_EQ(arrayExpr->elements.size(), 3);
}

void testArrayWithCall() {
    std::cout << "Testing Array with Call: [fixed(5)]..." << std::endl;
    // brr = [fixed(5)];
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "brr"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::LBRACKET, "["),
        Token(TokenType::IDENTIFIER, "fixed"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::NUMBER, "5"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::RBRACKET, "]"),
        Token(TokenType::SEMICOLON, ";")
    };

    auto program = parseTokens(tokens);
    ASSERT_NOT_NULL(program);
    auto& func = program->functions[0];
    auto* assignStmt = dynamic_cast<AssignmentStmt*>(func->body->statements[0].get());
    
    auto* arrayExpr = dynamic_cast<ArrayExpr*>(assignStmt->assignments[0].value.get());
    ASSERT_NOT_NULL(arrayExpr);
    ASSERT_EQ(arrayExpr->elements.size(), 1);
    
    auto* callExpr = dynamic_cast<CallExpr*>(arrayExpr->elements[0].get());
    ASSERT_NOT_NULL(callExpr);
    
    auto* callee = dynamic_cast<VarExpr*>(callExpr->callee.get());
    ASSERT_NOT_NULL(callee);
    ASSERT_EQ(callee->name, "fixed");
    ASSERT_EQ(callExpr->arguments.size(), 1);
}

void testMethodCall() {
    std::cout << "Testing Method Call: crr.push(i)..." << std::endl;
    
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "dummy"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::IDENTIFIER, "crr"),
        Token(TokenType::DOT, "."),
        Token(TokenType::IDENTIFIER, "push"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::IDENTIFIER, "i"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::SEMICOLON, ";")
    };

    auto program = parseTokens(tokens);
    ASSERT_NOT_NULL(program);
    auto& func = program->functions[0];
    auto* assignStmt = dynamic_cast<AssignmentStmt*>(func->body->statements[0].get());
    
    auto* methodCall = dynamic_cast<CallExpr*>(assignStmt->assignments[0].value.get());
    ASSERT_NOT_NULL(methodCall);
    
    auto* memberExpr = dynamic_cast<MemberExpr*>(methodCall->callee.get());
    ASSERT_NOT_NULL(memberExpr);
    ASSERT_EQ(memberExpr->name, "push");
    
    auto* obj = dynamic_cast<VarExpr*>(memberExpr->object.get());
    ASSERT_NOT_NULL(obj);
    ASSERT_EQ(obj->name, "crr");
}

void testNestedCalls() {
    std::cout << "Testing Nested Calls: [fixed(n, [fixed(n)])]..." << std::endl;
    // multidim2 = [fixed(n, [fixed(n)])];
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "multidim2"),
        Token(TokenType::EQUAL, "="),
        Token(TokenType::LBRACKET, "["),
        Token(TokenType::IDENTIFIER, "fixed"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::IDENTIFIER, "n"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::LBRACKET, "["),
        Token(TokenType::IDENTIFIER, "fixed"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::IDENTIFIER, "n"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::RBRACKET, "]"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::RBRACKET, "]"),
        Token(TokenType::SEMICOLON, ";")
    };

    auto program = parseTokens(tokens);
    ASSERT_NOT_NULL(program);
    auto& func = program->functions[0];
    auto* assignStmt = dynamic_cast<AssignmentStmt*>(func->body->statements[0].get());
    
    auto* arrayExpr = dynamic_cast<ArrayExpr*>(assignStmt->assignments[0].value.get());
    ASSERT_NOT_NULL(arrayExpr);
    
    // Outer fixed(...)
    auto* outerCall = dynamic_cast<CallExpr*>(arrayExpr->elements[0].get());
    ASSERT_NOT_NULL(outerCall);
    ASSERT_EQ(outerCall->arguments.size(), 2);
    
    // Second argument is inner array [fixed(n)]
    auto* innerArray = dynamic_cast<ArrayExpr*>(outerCall->arguments[1].get());
    ASSERT_NOT_NULL(innerArray);
    
    // Inner fixed(n)
    auto* innerCall = dynamic_cast<CallExpr*>(innerArray->elements[0].get());
    ASSERT_NOT_NULL(innerCall);
    ASSERT_EQ(innerCall->arguments.size(), 1);
}

int main() {
    std::cout << "Running Parser Array Tests..." << std::endl;
    testBasicArray();
    testArrayWithCall();
    testMethodCall();
    testNestedCalls();
    std::cout << "All array parser tests passed!" << std::endl;
    return 0;
}
