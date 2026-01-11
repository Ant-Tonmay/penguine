#include "lexer/lexer.h"
#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include <unordered_map>



Token::Token(TokenType type, const std::string& lexeme)
    : type(type), lexeme(lexeme) {}

std::string Token::toString() const {
    std::string typeStr;
    switch (type) {
        case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
        case TokenType::NUMBER: typeStr = "NUMBER"; break;
        case TokenType::PLUS: typeStr = "PLUS"; break;
        case TokenType::MINUS: typeStr = "MINUS"; break;
        case TokenType::STAR: typeStr = "STAR"; break;
        case TokenType::SLASH: typeStr = "SLASH"; break;
        case TokenType::EQUAL: typeStr = "EQUAL"; break;
        case TokenType::EQUAL_EQUAL: typeStr = "EQUAL_EQUAL"; break;
        case TokenType::LESS: typeStr = "LESS"; break;
        case TokenType::GREATER: typeStr = "GREATER"; break;
        case TokenType::LPAREN: typeStr = "LPAREN"; break;
        case TokenType::RPAREN: typeStr = "RPAREN"; break;
        case TokenType::KEYWORD: typeStr = "KEYWORD"; break;
        case TokenType::EOF_TOKEN: typeStr = "EOF"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return "Token(" + typeStr + ", \"" + lexeme + "\")";
}

Lexer::Lexer(const std::string& src)
    : source(src), current(0) {}




bool Lexer::isAtEnd() const {
    return current >= source.size();
}
char Lexer::advance() {
    return source[current++];
}
char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}
void Lexer::addToken(TokenType type, const std::string& lexeme) {
    tokens.emplace_back(type, lexeme);
}

void Lexer::number() {
    size_t start = current - 1;
    while (isdigit(peek())) advance();

    std::string value = source.substr(start, current - start);
    addToken(TokenType::NUMBER, value);
}

void Lexer::identifier() {
    size_t start = current - 1;
    while (isalpha(peek()) || peek() == '_') advance();

    std::string value = source.substr(start, current - start);
    if (keywords.count(value)) {
        addToken(TokenType::KEYWORD, value);
    } else {
        addToken(TokenType::IDENTIFIER, value);
    }
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        char c = advance();

        switch (c) {
            case '+': addToken(TokenType::PLUS, "+"); break;
            case '-': addToken(TokenType::MINUS, "-"); break;
            case '*': addToken(TokenType::STAR, "*"); break;
            case '/': addToken(TokenType::SLASH, "/"); break;
            case '=':
                if (peek() == '=') {
                    advance();
                    addToken(TokenType::EQUAL_EQUAL, "==");
                } else {
                    addToken(TokenType::EQUAL, "=");
                }
                break;
            case '<': addToken(TokenType::LESS, "<"); break;
            case '>': addToken(TokenType::GREATER, ">"); break;
            case '(' : addToken(TokenType::LPAREN, "("); break;
            case ')' : addToken(TokenType::RPAREN, ")"); break;

            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break; // ignore whitespace

            default:
                if (isdigit(c)) {
                    number();
                } else if (isalpha(c) || c == '_') {
                    identifier();
                }  else {
                    std::cerr << "Unexpected character: " << c << "\n";
                }
            }
        }

    addToken(TokenType::EOF_TOKEN, "");
    return tokens;
}

