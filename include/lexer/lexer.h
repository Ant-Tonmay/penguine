#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    PLUS,
    PLUS_EQUAL,
    MINUS,
    MINUS_EQUAL,
    STAR,
    STAR_EQUAL,
    SLASH,
    SLASH_EQUAL,
    MOD_OP,
    MOD_OP_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    NOT_EQUAL,
    BITWISE_AND,
    BITWISE_AND_EQUAL,
    BITWISE_OR,
    BITWISE_OR_EQUAL,
    BITWISE_XOR,
    XOR_EQUAL,
    OR,
    AND,
    NOT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    COLON,
    STRING,
    DOT,
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
