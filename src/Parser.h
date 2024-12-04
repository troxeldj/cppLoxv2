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

       std::shared_ptr<Expr> parse() {
              try {
                     return expression();
              } catch (const ParseError& e) {
                     return nullptr;
              }
       }

       std::shared_ptr<Expr> expression() {
              return equality();
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
