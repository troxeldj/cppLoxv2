#include <iostream>
#include "Lox.h"
#include "Util.h"
#include "Interpreter.h"
#include "Stmt.h"
#include "Scanner.h"
#include "Parser.h"

bool Lox::hadError = false;

void Lox::runFile(const std::string& path) {
    std::string fileContents = Util::fileToString(path);
    run(fileContents);

    if(hadError) std::exit(65);
}

void Lox::run(std::string source) {
    Scanner scanner{source};
    std::vector<Token> tokens = scanner.scanTokens();
    
    // Debug: Print tokens
    Parser parser{tokens};
    std::vector<std::shared_ptr<Stmt>> statements = parser.parse();

    if(hadError) return;

    Interpreter interpreter{};
    interpreter.interpret(statements);
}

void Lox::error(int line, const std::string& message) {
    report(line, "", message);
}

void Lox::report(int line, const std::string& where, const std::string& message) {
    std::cout << "[line " << line << "] " << where << ": " << message << std::endl;
}

void Lox::runPrompt() {
    std::cout << "> ";
    std::string line;
    while(true) {
        if(!std::getline(std::cin, line) || line == "exit") break;
        run(line);
        hadError = false;
    }
}

void Lox::error(const Token& token, const std::string& message) {
   if(token.type == END_OF_FILE) {
       report(token.line, " at end", message);
   } else {
       report(token.line, " at '" + token.lexeme + "'", message);
   }
}


int main(int argc, char *argv[]) {
    if(argc > 2) {
        std::cout << "Usage: jlox <script>\n";
    } else if (argc == 2) {
        Lox::runFile(argv[1]);
    } else {
       Lox::runPrompt();
    }
}
