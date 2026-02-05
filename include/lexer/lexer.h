#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,
    EQUAL_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    NOT_EQUAL,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    OR,
    AND,
    NOT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    STRING,
    KEYWORD,
    EOF_TOKEN
};


struct Token {
    TokenType type;
    std::string lexeme;

    Token(TokenType type, const std::string& lexeme);
    std::string toString() const;
};


class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::vector<Token> tokens;
    size_t current;
    bool isAtEnd() const;
    char advance();
    char peek() const;

    void addToken(TokenType type, const std::string& lexeme);
    void number();
    void identifier();
    void string();
};
