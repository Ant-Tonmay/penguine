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
  
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "print") {
        auto stmt = parsePrintStmt();
        consume(TokenType::SEMICOLON, "Expect ';' after print statement.");
        return stmt;
    }
    
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "for") {
       
        return parseForStmt();
    }
    
    if (check(TokenType::KEYWORD) && peek().lexeme == "for") {
        return parseForStmt();
    }

    if (check(TokenType::LBRACE)) {
        return parseBlock();
    }

    if (check(TokenType::KEYWORD) && peek().lexeme == "if") {
        return parseIfStmt();
    }
    
    auto stmt = parseAssignmentStmt();
    consume(TokenType::SEMICOLON, "Expect ';' after assignment statement.");
    return stmt;
}

std::unique_ptr<PrintStmt> Parser::parsePrintStmt() {
    
    advance(); 
    consume(TokenType::LPAREN, "Expect '(' after 'print'.");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after print value.");
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<AssignmentStmt> Parser::parseAssignmentStmt() {
    
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
    advance(); 
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
std::unique_ptr<IfStmt> Parser::parseIfStmt() {
    advance();
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after 'if' condition.");

    auto thenBranch = parseBlock();

    std::unique_ptr<Stmt> elseBranch = nullptr;


    if (match(TokenType::KEYWORD) && previous().lexeme == "else") {

        if (check(TokenType::KEYWORD) && peek().lexeme == "if") {
            elseBranch = parseIfStmt(); 
        } else {
            elseBranch = parseBlock();
        }
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}



std::unique_ptr<Expr> Parser::parseExpression() {
    return parseRelational();
}

std::unique_ptr<Expr> Parser::parseRelational() {

    auto left = parseAdditive();
    
    while (match(TokenType::LESS) || match(TokenType::GREATER) || match(TokenType::EQUAL_EQUAL)) {
        std::string op = previous().lexeme;
        auto right = parseAdditive();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    
    auto left = term();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = term();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    
    
    auto left = factor();
    
    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        std::string op = previous().lexeme;
        auto right = factor();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::factor() {
    
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
    

    throw std::runtime_error("Expect expression. Found: " + peek().lexeme);
}


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
