#include <map>
#include <any>
#include "Token.h"

class Environment {
  std::map<std::string, std::any> values;
  std::shared_ptr<Environment> enclosing;

public:
  Environment()
    : enclosing(nullptr) {} 

  Environment(std::shared_ptr<Environment> enclosing)
    : enclosing(std::move(enclosing)) {}

  void define(std::string name, std::any value) {
    values[name] = value;
  }

  std::any get(const Token& name) {
    if(values.find(name.lexeme) != values.end()) {
      return values[name.lexeme];
    }
    if(enclosing != nullptr) 
      return enclosing->get(name);
    throw new std::runtime_error("Undefined variable '" + name.lexeme + "'.");
  }

  void assign(const Token& name, std::any value) {
    if(values.find(name.lexeme) != values.end()) {
      values[name.lexeme] = value;
      return;
    }

    if(enclosing != nullptr) {
      enclosing->assign(name, value);
      return;
    }
    throw new std::runtime_error("Undefined variable '" + name.lexeme + "'.");
  }
};