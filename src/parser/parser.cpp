#include "parser/parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    
    consume(TokenType::LBRACE, "Expect '{' at start of program.");
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        program->functions.push_back(parseFunction());
    }
    
    consume(TokenType::RBRACE, "Expect '}' at end of program.");
    return program;
}

std::unique_ptr<Function> Parser::parseFunction() {
    // function -> "main" "(" ")" block
    // For now, grammar only specifies "main", but let's assume it can be any identifier or keyword "main"
    // Grammar: function -> "main" "(" ")" block
    // Let's be strict to grammar first: "main" is explicitly listed.
    // But lexer might type it as IDENTIFIER or KEYWORD?
    // "main" isn't in keywords map in Lexer, so it's IDENTIFIER "main".
    
    Token nameToken = consume(TokenType::IDENTIFIER, "Expect function name (e.g. 'main').");
    
    consume(TokenType::LPAREN, "Expect '(' after function name.");
    consume(TokenType::RPAREN, "Expect ')' after '('.");
    
    auto body = parseBlock();
    return std::make_unique<Function>(nameToken.lexeme, std::move(body));
}

std::unique_ptr<Block> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expect '{' to start block.");
    auto block = std::make_unique<Block>();
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
    }
    
    consume(TokenType::RBRACE, "Expect '}' to end block.");
    return block;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    // statement -> print_stmt ";" | assignment_stmt ";" | for_stmt | block
    
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "print") {
        auto stmt = parsePrintStmt();
        consume(TokenType::SEMICOLON, "Expect ';' after print statement.");
        return stmt;
    }
    
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "for") {
        // "for" might be keyword
        return parseForStmt();
    }
    
    // Check for KEYWORD "for" if not handled above (if Lexer changes)
    if (check(TokenType::KEYWORD) && peek().lexeme == "for") {
        return parseForStmt();
    }

    if (check(TokenType::LBRACE)) {
        return parseBlock();
    }
    
    // Default to assignment
    auto stmt = parseAssignmentStmt();
    consume(TokenType::SEMICOLON, "Expect ';' after assignment statement.");
    return stmt;
}

std::unique_ptr<PrintStmt> Parser::parsePrintStmt() {
    // print "(" expression ")"
    // consumes "print"
    advance(); 
    consume(TokenType::LPAREN, "Expect '(' after 'print'.");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after print value.");
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<AssignmentStmt> Parser::parseAssignmentStmt() {
    // assignment_stmt -> assignment ("," assignment)*
    // assignment -> IDENTIFIER "=" expression
    
    std::vector<Assignment> assignments;
    
    do {
        Token nameToken = consume(TokenType::IDENTIFIER, "Expect variable name.");
        consume(TokenType::EQUAL, "Expect '=' after variable name.");
        auto value = parseExpression();
        assignments.emplace_back(nameToken.lexeme, std::move(value));
    } while (match(TokenType::COMMA));
    
    return std::make_unique<AssignmentStmt>(std::move(assignments));
}

std::unique_ptr<ForStmt> Parser::parseForStmt() {
    // "for" "(" assignment_stmt ";" expression ";" assignment_stmt ")" block
    advance(); // consume "for"
    consume(TokenType::LPAREN, "Expect '(' after 'for'.");
    
    auto init = parseAssignmentStmt();
    consume(TokenType::SEMICOLON, "Expect ';' after loop initializer.");
    
    auto condition = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    
    auto increment = parseAssignmentStmt();
    consume(TokenType::RPAREN, "Expect ')' after loop clauses.");
    
    auto body = parseBlock();
    
    return std::make_unique<ForStmt>(std::move(init), std::move(condition), std::move(increment), std::move(body));
}

// ----------------
// Expressions
// ----------------

std::unique_ptr<Expr> Parser::parseExpression() {
    // expression -> term expression'
    // expression' -> ("+" | "-") term expression' | ε
    // Equivalent to: left = term(); while match(+,-) { right = term(); left = binary(left, op, right); }
    
    auto left = term();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = term();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    // term -> factor term'
    // term' -> ("*" | "/") factor term' | ε
    
    auto left = factor();
    
    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        std::string op = previous().lexeme;
        auto right = factor();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::factor() {
    // factor -> NUMBER | IDENTIFIER | "(" expression ")"
    
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberExpr>(previous().lexeme);
    }
    
    if (match(TokenType::STRING)) {
        return std::make_unique<StringExpr>(previous().lexeme);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VarExpr>(previous().lexeme);
    }
    
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return expr;
    }
    
    // Parse error
    throw std::runtime_error("Expect expression. Found: " + peek().lexeme);
}


// ----------------
// Helpers
// ----------------

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(message + " Found: " + peek().lexeme);
}
