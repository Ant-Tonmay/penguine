#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// --------------------
// Token types
// --------------------
enum class TokenType {
    IDENTIFIER,
    NUMBER,

    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,
    EQUAL_EQUAL,
    LESS,
    GREATER,

    // Punctuation
    LPAREN,
    RPAREN,

    KEYWORD,
    EOF_TOKEN
};

// --------------------
// Token structure
// --------------------
struct Token {
    TokenType type;
    std::string lexeme;

    Token(TokenType type, const std::string& lexeme);
    std::string toString() const;
};

// --------------------
// Lexer interface
// --------------------
class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::vector<Token> tokens;
    size_t current;

    // Helper functions
    bool isAtEnd() const;
    char advance();
    char peek() const;

    void addToken(TokenType type, const std::string& lexeme);
    void number();
    void identifier();

    // Keyword map
    std::unordered_map<std::string, TokenType> keywords = {
        {"if", TokenType::KEYWORD},
        {"else", TokenType::KEYWORD},
        {"while", TokenType::KEYWORD},
        {"return", TokenType::KEYWORD},
        {"func", TokenType::KEYWORD}
    };
};
