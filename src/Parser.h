//
// Created by dillon on 12/3/2024.
//

#ifndef PARSER_H
#define PARSER_H
#include <stdexcept>
#include <cassert>
#include <vector>
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include "Lox.h"


class Parser {
const std::vector<Token>& tokens;
int current = 0;

struct ParseError : public std::runtime_error {
       using std::runtime_error::runtime_error;
};
    
public:
       Parser(const std::vector<Token>& tokens)
       : tokens(std::move(tokens)) {}

       std::vector<std::shared_ptr<Stmt>> parse() {
              std::vector<std::shared_ptr<Stmt>> statements;
              while(!isAtEnd()) {
                     statements.push_back(declaration());
              }
              return statements;
       }

private:
       std::shared_ptr<Stmt> declaration() {
              try {
                     if(match(VAR)) return varDeclaration();
                     return statement();
              } catch(const ParseError& error) {
                     syncronize();
                     return nullptr;
              }
       }

       std::shared_ptr<Stmt> varDeclaration() {
              Token name = consume(IDENTIFIER, "Expect variable name.");
              std::shared_ptr<Expr> initializer = nullptr;
              if(match(EQUAL)) {
                     initializer = expression();
              }
              consume(SEMICOLON, "Expect ';' after variable declaration.");
              return std::make_shared<Var>(name, initializer);
       }

       std::shared_ptr<Stmt> statement() {
              if(match(IF)) return ifStatement();
              if(match(PRINT)) return printStatement();
              if(match(LEFT_BRACE)) return std::make_shared<Block>(block());
              return expressionStatement();
       }

       std::vector<std::shared_ptr<Stmt>> block() {
              std::vector<std::shared_ptr<Stmt>> statements;
              while(!check(RIGHT_BRACE) && !isAtEnd()) {
                     statements.push_back(declaration());
              }
              consume(RIGHT_BRACE, "Expect '}' after block.");
              return statements;
       }

       std::shared_ptr<Stmt> ifStatement() {
              consume(LEFT_PAREN, "Expect '(' after 'if'.");

              std::shared_ptr<Expr> condition = expression();
              
              consume(RIGHT_PAREN, "Expect ')' after if condition.");
              
              std::shared_ptr<Stmt> thenBranch = statement();
              std::shared_ptr<Stmt> elseBranch = nullptr;
              if(match(ELSE)) {
                     elseBranch = statement();
              }
              return std::make_shared<If>(condition, thenBranch, elseBranch);
       }
       


       std::shared_ptr<Stmt> printStatement() {
              std::shared_ptr<Expr> value = expression();
              consume(SEMICOLON, "Expect ';' after value.");
              return std::make_shared<Print>(value);
       }

       std::shared_ptr<Stmt> expressionStatement() {
              std::shared_ptr<Expr> expr = expression();
              consume(SEMICOLON, "Expect ';' after expression.");
              return std::make_shared<Expression>(expr);
       }

       std::shared_ptr<Expr> expression() {
              return assignment();
       }

       std::shared_ptr<Expr> assignment() {
              std::shared_ptr<Expr> expr = orOp();
              if(match(EQUAL)) {
                     Token equals = previous();
                     std::shared_ptr<Expr> value = assignment();
                     if(std::shared_ptr<Variable> var = std::dynamic_pointer_cast<Variable>(expr)) {
                            Token name = var->name;
                            return std::make_shared<Assign>(name, value);
                     }
                     error(equals, "Invalid assignment target.");
              }
              return expr;
       }

       std::shared_ptr<Expr> orOp() {
              std::shared_ptr<Expr> expr = andOp();
              while(match(OR)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = andOp();
                     expr = std::make_shared<Logical>(expr, op, right);
              }
              return expr;
       }

       std::shared_ptr<Expr> andOp() {
              std::shared_ptr<Expr> expr = equality();
              while(match(AND)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = equality();
                     expr = std::make_shared<Logical>(expr, op, right);
              }
              return expr;
       }

       std::shared_ptr<Expr> equality() {
              std::shared_ptr<Expr> expr = comparison();
              while(match(BANG_EQUAL, EQUAL_EQUAL)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = comparison();
                     expr = std::make_shared<Binary>(expr , std::move(op), right);
              }
              return expr;
       }

       std::shared_ptr<Expr> comparison() {
              std::shared_ptr<Expr> expr = term();
              while(match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = term();
                     expr = std::make_shared<Binary>(expr , std::move(op), right);
              }
              return expr;
       }

       std::shared_ptr<Expr> term() {
              std::shared_ptr<Expr> expr = factor();
              while(match(MINUS, PLUS)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = factor();
                     expr = std::make_shared<Binary>(expr , std::move(op), right);
              }
              return expr;
       }

       std::shared_ptr<Expr> factor() {
              std::shared_ptr<Expr> expr = unary();
              while(match(SLASH, STAR)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = unary();
                     expr = std::make_shared<Binary>(expr , std::move(op), right);
              }
              return expr;
       }

       std::shared_ptr<Expr> unary() {
              if(match(BANG, MINUS)) {
                     Token op = previous();
                     std::shared_ptr<Expr> right = unary();
                     return std::make_shared<Unary>(std::move(op), right);
              }
              return primary();
       }

       std::shared_ptr<Expr> primary() {
              if(match(FALSE)) return std::make_shared<Literal>(false);
              if(match(TRUE)) return std::make_shared<Literal>(true);
              if(match(NIL)) return std::make_shared<Literal>(nullptr);

              if(match(NUMBER, STRING))
                     return std::make_shared<Literal>(previous().literal);
              
              if(match(IDENTIFIER))
                     return std::make_shared<Variable>(previous());

              if(match(LEFT_PAREN)) {
                     std::shared_ptr<Expr> expr = expression();
                     consume(RIGHT_PAREN, "Expect ')' after expression.");
                     return std::make_shared<Grouping>(expr);
              }

              throw error(peek(), "Expected Expression.");
       }

       template <class... T>
       bool match(T... type) {
              assert((... && std::is_same_v<T, TokenType>));
              if((... || check(type))) {
                     advance();
                     return true;
              }
              return false;
       }

       Token consume(TokenType type, const std::string& message) {
              if(check(type)) return advance();
              throw error(peek(), message);
       }

       bool check(TokenType type) {
              if(isAtEnd()) return false;
              return peek().type == type;
       }

       bool isAtEnd() {
              return peek().type == END_OF_FILE;
       }

       Token advance() {
              if(!isAtEnd()) ++current;
              return previous();
       }

       Token peek() {
              return tokens.at(current);
       }

       Token previous() {
              return tokens.at(current-1);
       }

       static ParseError error(const Token& token, const std::string& message) {
              Lox::error(token, message);
              return ParseError{""};
       }

       void syncronize() {
              advance();
              while(!isAtEnd()) {
                     if(previous().type == SEMICOLON) return;
                     switch(peek().type) {
                            case CLASS:
                            case FUN:
                            case VAR:
                            case FOR:
                            case IF:
                            case WHILE:
                            case PRINT:
                            case RETURN:
                                   return;
                            default:
                                   advance();
                     }
              }
       }
};
#endif //PARSER_H
