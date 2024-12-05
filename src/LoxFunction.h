#ifndef LOX_FUNCTION_H
#define LOX_FUNCTION_H 
#include <memory>
#include <any>
#include <string>
#include <vector>
#include "LoxCallable.h"

class Environment;
class Function;


class LoxFunction : public LoxCallable {
  std::shared_ptr<Function> declaration;
  std::shared_ptr<Environment> closure;

public:
  LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure); 
  std::string toString() override;
  std::any call(Interpreter& interpreter, std::vector<std::any> arguments) override;
  int arity() override;
};

#endif //LOX_FUNCTION_H