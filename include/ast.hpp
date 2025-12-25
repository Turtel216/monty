#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ggc {
class ExprAST {
public:
  virtual ~ExprAST() = default;
};

class NumberExprAST : ExprAST {
  double val;

public:
  NumberExprAST(double _val) noexcept : val(_val) {}
};

class VariableExprAST : ExprAST {
  std::string name;

public:
  VariableExprAST(const std::string &_name) noexcept : name(_name) {}
};

class BinaryExprAST : ExprAST {
  char op;
  std::unique_ptr<ExprAST> Lhs, Rhs;

public:
  BinaryExprAST(char _op, std::unique_ptr<ExprAST> _Lhs,
                std::unique_ptr<ExprAST> _Rhs) noexcept
      : op(_op), Lhs(std::move(_Lhs)), Rhs(std::move(_Rhs)) {}
};

class FunctionCallExprAST : ExprAST {
  std::string caller;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  FunctionCallExprAST(const std::string &_caller,
                      std::vector<std::unique_ptr<ExprAST>> _args) noexcept
      : caller(_caller), args(std::move(_args)) {};
};

// Represents a functions declaration
class FunctionPrototypeAST {
  std::string name;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  FunctionPrototypeAST(const std::string &_name,
                       std::vector<std::unique_ptr<ExprAST>> _args) noexcept
      : name(_name), args(std::move(_args)) {};

  std::string getName() const noexcept { return name; }
};

class FunctionAST {
  std::unique_ptr<FunctionPrototypeAST> prototype;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<FunctionPrototypeAST> _prototype,
              std::unique_ptr<ExprAST> _body) noexcept
      : prototype(std::move(_prototype)), body(std::move(_body)) {}
};
} // namespace ggc
