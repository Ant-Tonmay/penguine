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
    if (match(TokenType::KEYWORD) && previous().lexeme == "return") {
        std::unique_ptr<Expr> value = nullptr;

        if (!check(TokenType::SEMICOLON)) {
            value = parseExpression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after return");
        return std::make_unique<ReturnStmt>(std::move(value));
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
        auto target = std::make_unique<VarExpr>(nameToken.lexeme);
        assignments.emplace_back(std::move(target), std::move(value));
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
    return parseLogicalOr();
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();

    while (match(TokenType::OR)) { // ||
        std::string op = previous().lexeme;
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}


std::unique_ptr<Expr> Parser::parseBitwiseAnd(){
    auto left = parseEquality();
    while(match(TokenType::BITWISE_AND)) {
        std::string op = previous().lexeme;
        auto right = parseEquality();
        left = std::make_unique<BinaryExpr>(std::move(left),op,std::move(right));
    }
    return left;
}
std::unique_ptr<Expr> Parser::parseBitwiseXor(){
    auto left = parseBitwiseAnd();
    while(match(TokenType::BITWISE_XOR)){
        std::string op = previous().lexeme;
        auto right = parseBitwiseAnd();
        left = std::make_unique<BinaryExpr>(std::move(left),op,std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseOr(){
    auto left = parseBitwiseXor();
    while(match(TokenType::BITWISE_OR)){
        std::string op = previous().lexeme;
        auto right = parseBitwiseXor();
        left = std::make_unique<BinaryExpr>(std::move(left),op,std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto left = parseBitwiseOr();

    while (match(TokenType::AND)) { // &&
        std::string op = previous().lexeme;
        auto right = parseBitwiseOr();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}
std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseComparison();

    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::NOT_EQUAL)) {
        std::string op = previous().lexeme;
        auto right = parseComparison();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}
std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseShift();

    while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) ||
           match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL)) {
        std::string op = previous().lexeme;
        auto right = parseShift();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}
std::unique_ptr<Expr> Parser::parseShift() {
    auto left = parseAdditive();

    while (match(TokenType::LEFT_SHIFT) || match(TokenType::RIGHT_SHIFT)) {
        std::string op = previous().lexeme;
        auto right = parseAdditive();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}
std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    
    auto left = parseMultiplicative();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto left = parseUnary();

    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        std::string op = previous().lexeme;
        auto right = parseUnary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (match(TokenType::LBRACKET)) {
            auto index = parseExpression();
            consume(TokenType::RBRACKET, "Expect ']'.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        }
        else if (match(TokenType::LPAREN)) {
            std::vector<std::unique_ptr<Expr>> arguments;
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments));
        }
        else if (match(TokenType::DOT)) {
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::make_unique<MemberExpr>(std::move(expr), name.lexeme);
        }
        else {
            break;
        }
    }

    return expr;
}


std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::LBRACKET)) {
        std::vector<std::unique_ptr<Expr>> elements;

        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(parseExpression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACKET, "Expect ']' after array literal.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }


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

    throw std::runtime_error("Expect expression.");
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
std::vector<Param> Parser::parseParams(){

    std::vector<Param> params;
    if (check(TokenType::RPAREN)) return params;
     do {
        bool isRef = false;

        // Detect ref:
        if (check(TokenType::IDENTIFIER) && peek().lexeme == "ref") {
            advance(); // consume "ref"
            consume(TokenType::COLON, "Expected ':' after 'ref'");
            isRef = true;
        }

        consume(TokenType::IDENTIFIER, "Expected parameter name");
        std::string name = previous().lexeme;

        params.emplace_back(name, isRef);

    } while (match(TokenType::COMMA));

    return params;
}

std::unique_ptr<Function> Parser::parseFunction() {
    consume(TokenType::KEYWORD, "Expected 'func'");
    consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = previous().lexeme;

    consume(TokenType::LPAREN, "Expected '(' after function name");
    auto params = parseParams();
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    auto body = parseBlock();

    return std::make_unique<Function>(name, std::move(params), std::move(body));
}

