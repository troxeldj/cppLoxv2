#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <stdexcept>
#include <iostream>
#include "Expr.h"
#include "Stmt.h"
#include "Environment.h"

class Interpreter : public ExprVisitor, public StmtVisitor {
  std::shared_ptr<Environment> environment{new Environment};
public:

  Interpreter() = default;

  void interpret(std::vector<std::shared_ptr<Stmt>>& statements) {
    try {
      for(std::shared_ptr<Stmt>& stmt : statements) {
        execute(stmt);
      }
    } catch(const std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }

private:
  std::any evaluate(std::shared_ptr<Expr> expr) {
    return expr->accept(*this);
  }

  void execute(std::shared_ptr<Stmt> stmt) {
    stmt->accept(*this);
  }

  std::any visitIfStmt(std::shared_ptr<If> stmt) override {
    if(isTruthy(evaluate(stmt->condition))) {
      execute(stmt->thenBranch);
    } else if(stmt->elseBranch != nullptr) {
      execute(stmt->elseBranch);
    }
    return {};
  }

  std::any visitWhileStmt(std::shared_ptr<While> stmt) override {
    while(isTruthy(evaluate(stmt->condition))) {
      execute(stmt->body);
    }
    return {};
  }

  std::any visitBlockStmt(std::shared_ptr<Block> stmt) override {
    executeBlock(stmt->statements, std::make_shared<Environment>(environment));
    return {};
  }

  void executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> environment) {
    std::shared_ptr<Environment> previous = this->environment;
    try {
      this->environment = environment;
      for (std::shared_ptr<Stmt>& stmt : statements) {
        execute(stmt);
      }
    } catch (...) {
      this->environment = previous;
      throw;
    }
    this->environment = previous;
  }

  std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) override {
    evaluate(stmt->expression);
    return {};
  }

  std::any visitPrintStmt(std::shared_ptr<Print> stmt) override {
    std::any value = evaluate(stmt->expression);
    std::cout << stringify(value) << std::endl;
    return {};
  }

  std::any visitVarStmt(std::shared_ptr<Var> stmt) override {
    std::any value = nullptr;
    if(stmt->initializer != nullptr) {
      value = evaluate(stmt->initializer);
    }
    environment->define(stmt->name.lexeme, value);
    return {};
  }

  std::any visitAssignExpr(std::shared_ptr<Assign> expr) override {
    std::any value = evaluate(expr->value);
    environment->assign(expr->name, value);
    return value;
  }

  std::any visitBinaryExpr(std::shared_ptr<Binary> expr) override {
   std::any left = expr->left->accept(*this); 
   std::any right = expr->right->accept(*this);

   switch(expr->op.type) {
    case BANG_EQUAL: return !isEqual(left, right); break;
    case EQUAL_EQUAL: return isEqual(left, right); break;
    case GREATER:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) > std::any_cast<double>(right);
      break;
    case GREATER_EQUAL:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) >= std::any_cast<double>(right);
      break;
    case LESS:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) < std::any_cast<double>(right);
      break;
    case LESS_EQUAL:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) <= std::any_cast<double>(right);
      break;
    case PLUS:
      if(left.type() == typeid(double) && right.type() == typeid(double)) {
        return std::any_cast<double>(left) + std::any_cast<double>(right);
      } else if(left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
        return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
      } else {
        throw std::runtime_error("Operands must be two numbers or two strings.");
      }
    case MINUS:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) - std::any_cast<double>(right);
    case SLASH:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) / std::any_cast<double>(right);
    case STAR:
      checkNumberOperands(expr->op, left, right);
      return std::any_cast<double>(left) * std::any_cast<double>(right);
   }
   return {};
  }

  std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) override {
    return evaluate(expr->expression);
  }

  std::any visitLiteralExpr(std::shared_ptr<Literal> expr) override {
    return expr->value;
  }

  std::any visitUnaryExpr(std::shared_ptr<Unary> expr) override {
    std::any right = expr->right->accept(*this);
    switch(expr->op.type) {
      case MINUS:
        checkNumberOperand(expr->op, right);
        return -std::any_cast<double>(right);
      case BANG:
        return !isTruthy(right);
    }
    return {};
  }

  std::any visitVariableExpr(std::shared_ptr<Variable> expr) override {
    return environment->get(expr->name);
  }

  std::any visitLogicalExpr(std::shared_ptr<Logical> expr) override {
    std::any left = evaluate(expr->left);
    if(expr->op.type == OR) {
      if(isTruthy(left)) return left;
    } else {
      if(!isTruthy(left)) return left;
    }
    return evaluate(expr->right);
  }

  

  bool isEqual(std::any left, std::any right) {
    if(left.type() == typeid(nullptr) && right.type() == typeid(nullptr)) return true;

    if(left.type() == typeid(nullptr)) return false;

    if(left.type() == typeid(double) && right.type() == typeid(double)) {
      return std::any_cast<double>(left) == std::any_cast<double>(right);
    } else if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
      return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
    } else if (left.type() == typeid(bool) && right.type() == typeid(bool)) {
      return std::any_cast<bool>(left) == std::any_cast<bool>(right);
    }
    return false;
  }

  std::string stringify(std::any value) {
    if(value.type() == typeid(nullptr)) return "nil";
    if(value.type() == typeid(double)) return std::to_string(std::any_cast<double>(value));
    if(value.type() == typeid(std::string)) return std::any_cast<std::string>(value);
    if(value.type() == typeid(bool)) return std::any_cast<bool>(value) ? "true" : "false";
    return "Error in stringify. Value type not recognized.";
  }


  void checkNumberOperand(const Token& op, std::any& operand) {
    if(operand.type() == typeid(double)) return;
    throw std::runtime_error("Operand must be a number.");
  }

  void checkNumberOperands(const Token& op, std::any& left, std::any& right) {
    if(left.type() == typeid(double) && right.type() == typeid(double)) return;
    throw std::runtime_error("Operands must be numbers.");   
  }

  bool isTruthy(std::any value) {
    if(value.type() == typeid(nullptr)) return false;
    if(value.type() == typeid(bool)) return std::any_cast<bool>(value);
    return true;
  }
};

#endif //INTERPRETER_H