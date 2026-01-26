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
        case TokenType::LBRACE: typeStr = "LBRACE"; break;
        case TokenType::RBRACE: typeStr = "RBRACE"; break;
        case TokenType::SEMICOLON: typeStr = "SEMICOLON"; break;
        case TokenType::COMMA: typeStr = "COMMA"; break;
        case TokenType::STRING: typeStr = "STRING"; break;
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

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            // simpler handling for now, maybe error?
        }
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string." << std::endl;
        return;
    }

    advance(); // The closing "

    // Trim quotes
    // start was the quote
    // current is one past the closing quote
    // so we want from start+1, length = (current-1) - (start+1) = current - start - 2
    
    // Actually our helper structure is a bit different. Let's just capture content.
    // The previous token logic was "start = current - 1" inside number/id calls.
    // But here we are called AFTER consuming the first quote.
    // So let's rely on extracting from source relative to where we assume we are.
    // Wait, let's look at how tokenize calls this. tokenize consumes '"' then calls string().
    // So `current` is past the first quote.
    
    // Let's refine the logic to match `number()` pattern or adjust `tokenize`.
    // `number()` assumes first digit already consumed/peeked?
    // No, `tokenize` peeks via switch/default.
    // `number()`: start = current - 1. `tokenize` calls `number()` inside `default`. 
    // Is `c` consumed in `tokenize`? Yes `char c = advance();`
    
    // So for string:
    size_t start = current - 1; // Includes opening quote
    while (peek() != '"' && !isAtEnd()) {
        advance();
    }
    
    if (isAtEnd()) {
         std::cerr << "Unterminated string." << std::endl;
         return;
    }
    
    advance(); // Closing quote
    
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::identifier() {
    size_t start = current - 1;
    while (isalpha(peek()) || peek() == '_') advance();

    std::string value = source.substr(start, current - start);
    
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"if", TokenType::KEYWORD},
        {"else", TokenType::KEYWORD},
        {"while", TokenType::KEYWORD},
        {"return", TokenType::KEYWORD},
        {"func", TokenType::KEYWORD}
    };

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
            case '(': addToken(TokenType::LPAREN, "("); break;
            case ')': addToken(TokenType::RPAREN, ")"); break;
            case '{': addToken(TokenType::LBRACE, "{"); break;
            case '}': addToken(TokenType::RBRACE, "}"); break;
            case ';': addToken(TokenType::SEMICOLON, ";"); break;
            case ',': addToken(TokenType::COMMA, ","); break;
            case '"': string(); break;

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

