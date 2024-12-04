//
// Created by dillon on 12/3/2024.
//

#ifndef LOX_H
#define LOX_H
#include <string>

struct Token;

class Lox {
public:
    static bool hadError;
    static void runFile(const std::string&);
    static void run(std::string);
    static void error(int line, const std::string& );
    static void error(const Token&, const std::string&);
    static void report(int, const std::string&, const std::string&);
    static void runPrompt();
};
#endif //LOX_H
