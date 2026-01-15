#include "parser/parser.h"
#include <stdexcept>

// --------------------
// Constructor
// --------------------
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

// --------------------
// Entry point
// --------------------
std::unique_ptr<Expr> Parser::parse() {
    auto expr = expression();
    if (!isAtEnd()) {
        throw std::runtime_error("Unexpected token after expression: " + peek().lexeme);
    }
    return expr;
}

// --------------------
// Grammar rules
// expression → term ( ( "+" | "-" ) term )*
// --------------------
std::unique_ptr<Expr> Parser::expression() {
    auto expr = term();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = term();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), op, std::move(right)
        );
    }

    return expr;
}

// --------------------
// term → factor ( ( "*" | "/" ) factor )*
// --------------------
std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();

    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        std::string op = previous().lexeme;
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), op, std::move(right)
        );
    }

    return expr;
}

// --------------------
// factor → NUMBER | IDENTIFIER | "(" expression ")"
// --------------------
std::unique_ptr<Expr> Parser::factor() {
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberExpr>(previous().lexeme);
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierExpr>(previous().lexeme);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        if (!match(TokenType::RPAREN)) {
            throw std::runtime_error("Expected ')'");
        }
        return expr;
    }

    throw std::runtime_error("Unexpected token: " + peek().lexeme);
}

// --------------------
// Helper functions
// --------------------
bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current++;
        return previous();
    }
    return peek();
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}
