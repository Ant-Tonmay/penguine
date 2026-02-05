#include "../include/lexer/lexer.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

#define ASSERT_TOKEN(token, expectedType, expectedLexeme) \
    do { \
        if ((token).type != (expectedType) || (token).lexeme != (expectedLexeme)) { \
            std::cerr << "Test failed at " << __LINE__ << ": Expected " << (expectedLexeme) \
                      << ", got " << (token).lexeme << std::endl; \
            exit(1); \
        } \
    } while(0)

void testBasics() {
    Lexer lexer("123 + 456");
    auto tokens = lexer.tokenize();

    ASSERT_TOKEN(tokens[0], TokenType::NUMBER, "123");
    ASSERT_TOKEN(tokens[1], TokenType::PLUS, "+");
    ASSERT_TOKEN(tokens[2], TokenType::NUMBER, "456");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    
    std::cout << "testBasics passed" << std::endl;
}

void testKeywords() {
    Lexer lexer("if else while return func");
    auto tokens = lexer.tokenize();

    ASSERT_TOKEN(tokens[0], TokenType::KEYWORD, "if");
    ASSERT_TOKEN(tokens[1], TokenType::KEYWORD, "else");
    ASSERT_TOKEN(tokens[2], TokenType::KEYWORD, "while");
    ASSERT_TOKEN(tokens[3], TokenType::KEYWORD, "return");
    ASSERT_TOKEN(tokens[4], TokenType::KEYWORD, "func");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");

    std::cout << "testKeywords passed" << std::endl;
}

void testIdentifiers() {
    Lexer lexer("foo bar_baz");
    auto tokens = lexer.tokenize();

    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "foo");
    ASSERT_TOKEN(tokens[1], TokenType::IDENTIFIER, "bar_baz");
    ASSERT_TOKEN(tokens[2], TokenType::EOF_TOKEN, "");

    std::cout << "testIdentifiers passed" << std::endl;
}

void testComplex() {
    Lexer lexer("return a + 5");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::KEYWORD, "return");
    ASSERT_TOKEN(tokens[1], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[2], TokenType::PLUS, "+");
    ASSERT_TOKEN(tokens[3], TokenType::NUMBER, "5");
    std::cout << "testComplex passed" << std::endl;
}

void test_and(){
    Lexer lexer(" a && b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::AND, "&&");  
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    std::cout << "test_and passed" << std::endl;
}

void test_or(){
    Lexer lexer(" a || b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::OR, "||");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    std::cout << "test_or passed" << std::endl;
}

void test_not_equal(){
    Lexer lexer(" a != b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::NOT_EQUAL, "!=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    std::cout << "test_not_equal passed" << std::endl;
}

void test_less_equal(){
    Lexer lexer(" a <= b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::LESS_EQUAL, "<=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    std::cout << "test_less_equal passed" << std::endl;
}

void test_greater_equal(){
    Lexer lexer(" a >= b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::GREATER_EQUAL, ">=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[3], TokenType::EOF_TOKEN, "");
    std::cout << "test_greater_equal passed" << std::endl;
}

void test_bitwise_and(){
    Lexer lexer(" x = a & b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[3], TokenType::BITWISE_AND, "&");
    ASSERT_TOKEN(tokens[4], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");
    std::cout << "test_bitwise_and passed" << std::endl;
}

void test_bitwise_or(){
    Lexer lexer(" x = a | b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[3], TokenType::BITWISE_OR, "|");
    ASSERT_TOKEN(tokens[4], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");
    std::cout << "test_bitwise_or passed" << std::endl;
}

void test_bitwise_xor(){
    Lexer lexer(" x = a ^ b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[3], TokenType::BITWISE_XOR, "^");
    ASSERT_TOKEN(tokens[4], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");
    std::cout << "test_bitwise_xor passed" << std::endl;
}

void test_not(){
    Lexer lexer("a = !b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::NOT, "!");
    ASSERT_TOKEN(tokens[3], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[4], TokenType::EOF_TOKEN, "");
    std::cout << "test_not passed" << std::endl;
}

void test_left_shift(){
    Lexer lexer("x = a << b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[3], TokenType::LEFT_SHIFT, "<<");
    ASSERT_TOKEN(tokens[4], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");
    std::cout << "test_left_shift passed" << std::endl;
}

void test_right_shift(){
    Lexer lexer("x = a >> b");
    auto tokens = lexer.tokenize();
    ASSERT_TOKEN(tokens[0], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN(tokens[1], TokenType::EQUAL, "=");
    ASSERT_TOKEN(tokens[2], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN(tokens[3], TokenType::RIGHT_SHIFT, ">>");
    ASSERT_TOKEN(tokens[4], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN(tokens[5], TokenType::EOF_TOKEN, "");
    std::cout << "test_right_shift passed" << std::endl;
}

int main() {
    std::cout << "Running Lexer Tests..." << std::endl;
    testBasics();
    testKeywords();
    testIdentifiers();
    testComplex();
    test_and();
    test_or();
    test_not();
    test_not_equal();
    test_less_equal();
    test_greater_equal();
    test_bitwise_and();
    test_bitwise_or();
    test_bitwise_xor(); 
    test_left_shift();
    test_right_shift();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
