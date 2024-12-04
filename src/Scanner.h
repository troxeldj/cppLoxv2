//
// Created by dillon on 12/3/2024.
//

#ifndef SCANNER_H
#define SCANNER_H
#include <vector>
#include <map>
#include "Token.h"
#include "Lox.h"

class Scanner {
private:
    static const std::map<std::string, TokenType> keywords;
    
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;
public:

    Scanner(std::string source)
        : source(std::move(source)) {}

    std::vector<Token> scanTokens() {
        while(!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.push_back(Token(END_OF_FILE, "", nullptr, line));
        return tokens;
    }

private:
    void scanToken() {
      char c = advance();
      switch (c) {
        case '(': addToken(LEFT_PAREN); break;
        case ')': addToken(RIGHT_PAREN); break;
        case '{': addToken(LEFT_BRACE); break;
        case '}': addToken(RIGHT_BRACE); break;
        case ',': addToken(COMMA); break;
        case '-': addToken(MINUS); break;
        case '+': addToken(PLUS); break;
        case '*': addToken(STAR); break;

        case '!':
          addToken(match('=') ? BANG_EQUAL : BANG);
          break;

        case '=':
          addToken(match('=') ? EQUAL_EQUAL : EQUAL);
          break;

        case '<':
          addToken(match('=') ? LESS_EQUAL : LESS);
          break;

        case '>':
          addToken(match('=') ? GREATER_EQUAL : GREATER);
          break;

        case '/':
          if(match('/')) {
             while(peek() != '\n' && !isAtEnd()) advance();
          } else {
             addToken(SLASH);
          }
        break;

        case ' ':
        case '\r':
        case '\t':
          break;

        case '\n':
          line++; break;

          case '"': string(); break;

        default:
          if(isdigit(c)) {
            number();
          } else if(isalpha(c)) {
            identifier();
          } else {
            Lox::error(line, "Unexpected character.");
          }
          break;
      }
    }

    void number() {
        while(isDigit(peek())) advance();
        if(peek() == '.' && isDigit(peekNext())) {
            advance();
            while(isDigit(peek())) advance();
        }
        addToken(NUMBER, std::stod(std::string{source.substr(start, current - start)}));
    }

    void string() {
        while(peek() != '"' && !isAtEnd()) {
            if(peek() == '\n') line++;
            advance();
        }

        if(isAtEnd()) {
            Lox::error(line, "Unterminated string.");
            return;
        }
        advance();

        std::string value = std::string{source.substr(start + 1, current - 2 - start)};
        addToken(STRING, value);
    }


    void identifier() {
        while(isAlphaNumeric(peek())) advance();
        std::string text = std::string{source.substr(start, current - start)};

        TokenType type;
        auto match = keywords.find(text);
        if(match == keywords.end()) {
            type = IDENTIFIER;
        } else {
            type = match->second;
        }
        addToken(type);
    }

    bool match(char expected) {
      if(isAtEnd()) return false;
        if(source[current] != expected) return false;
        ++current;
        return true;
    }

    bool isAtEnd() {
        return current >= source.length();
    }

    char advance() {
        return source[current++];
    }

    char peekNext() {
        if(current + 1 >= source.size()) return '\0';
        return source[current + 1];
    }

    char peek() {
        if(isAtEnd()) return '\0';
        return source[current];
    }

    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }

    void addToken(TokenType type) {
      addToken(type, nullptr);
    }

    void addToken(TokenType type, std::any literal) {
        std::string text = source.substr(start, current - start);
        Token token{type, std::move(text), std::move(literal), line};
        tokens.push_back(token);
    }
};

const std::map<std::string, TokenType> Scanner::keywords = {
    {"and",    TokenType::AND},
     {"class",  TokenType::CLASS},
     {"else",   TokenType::ELSE},
     {"false",  TokenType::FALSE},
     {"for",    TokenType::FOR},
     {"fun",    TokenType::FUN},
     {"if",     TokenType::IF},
     {"nil",    TokenType::NIL},
     {"or",     TokenType::OR},
     {"print",  TokenType::PRINT},
     {"return", TokenType::RETURN},
     {"super",  TokenType::SUPER},
     {"this",   TokenType::THIS},
     {"true",   TokenType::TRUE},
     {"var",    TokenType::VAR},
     {"while",  TokenType::WHILE},
};

#endif //SCANNER_H
