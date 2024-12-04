//
// Created by dillon on 12/3/2024.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <any>
#include "TokenType.h"

class Token {
public:
    TokenType type;
    std::string lexeme;
    std::any literal;
    int line;

    Token(TokenType type, std::string lexeme, std::any literal, int line) 
        : type{type}, lexeme{std::move(lexeme)}, 
        literal{std::move(literal)}, line{line} {}

    Token(const Token& other)
        : type{other.type}, lexeme{std::move(other.lexeme)}, 
        literal{std::move(other.literal)}, line{other.line} {}
    
    Token(Token&& other) 
        : type{other.type}, lexeme{std::move(other.lexeme)}, 
        literal{std::move(other.literal)}, line{other.line}   {}

    std::string toString() {
        std::string literalText;
        switch (type) {
            case IDENTIFIER:
                literalText = lexeme;
            break;
            case STRING:
                literalText = std::any_cast<std::string>(literal);
            break;
            case NUMBER:
                literalText = std::any_cast<std::string>(std::any_cast<double>(literal));
            break;
            case TRUE:
                literalText = "true";
            break;
            case FALSE:
                literalText = "false";
            break;
            default:
                literalText = "nil";
        }
        return ::toString(type) + " " + lexeme + " " + literalText;
    }
};
#endif //TOKEN_H
