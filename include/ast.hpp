#pragma once

#include "generator.hpp"
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <vector>

namespace monty {
class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen(GeneratorContext &context) const noexcept = 0;
};

class NumberExprAST : public ExprAST {
  double val;

public:
  NumberExprAST(double _val) noexcept : val(_val) {}
  virtual llvm::Value *
  codegen(GeneratorContext &context) const noexcept override;
};

class VariableExprAST : public ExprAST {
  std::string name;

public:
  VariableExprAST(const std::string &_name) noexcept : name(_name) {}

  virtual llvm::Value *
  codegen(GeneratorContext &context) const noexcept override;
};

class BinaryExprAST : public ExprAST {
  char op;
  std::unique_ptr<ExprAST> Lhs, Rhs;

public:
  BinaryExprAST(char _op, std::unique_ptr<ExprAST> _Lhs,
                std::unique_ptr<ExprAST> _Rhs) noexcept
      : op(_op), Lhs(std::move(_Lhs)), Rhs(std::move(_Rhs)) {}

  virtual llvm::Value *
  codegen(GeneratorContext &context) const noexcept override;
};

class FunctionCallExprAST : public ExprAST {
  std::string caller;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  FunctionCallExprAST(const std::string &_caller,
                      std::vector<std::unique_ptr<ExprAST>> _args) noexcept
      : caller(_caller), args(std::move(_args)) {};

  virtual llvm::Value *
  codegen(GeneratorContext &context) const noexcept override;
};

// Represents a functions declaration
class FunctionPrototypeAST {
  std::string name;
  std::vector<std::string> args;

public:
  FunctionPrototypeAST(const std::string &_name,
                       std::vector<std::string> _args) noexcept
      : name(_name), args(std::move(_args)) {};

  std::string getName() const noexcept { return name; }

  llvm::Function *codegen(GeneratorContext &context) const noexcept;
};

class FunctionAST {
  std::unique_ptr<FunctionPrototypeAST> prototype;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<FunctionPrototypeAST> _prototype,
              std::unique_ptr<ExprAST> _body) noexcept
      : prototype(std::move(_prototype)), body(std::move(_body)) {}

  llvm::Function *codegen(GeneratorContext &context) const noexcept;
};
} // namespace monty
