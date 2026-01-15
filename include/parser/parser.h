#pragma once

#include <vector>
#include <memory>

#include "lexer/lexer.h"
#include "parser/ast.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Expr> parse();

private:
    const std::vector<Token>& tokens;
    size_t current;

    // Grammar rules
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();

    // Helpers
    bool match(TokenType type);
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
};
