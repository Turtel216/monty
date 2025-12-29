#pragma once

#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <vector>

namespace monty {

// Forward declartion
class NumberExprAST;
class VariableExprAST;
class BinaryExprAST;
class UnaryExprAST;
class IfExprAST;
class FunctionCallExprAST;
class FunctionPrototypeAST;
class FunctionAST;

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  virtual void visit(const NumberExprAST &node) = 0;
  virtual void visit(const VariableExprAST &node) = 0;
  virtual void visit(const BinaryExprAST &node) = 0;
  virtual void visit(const UnaryExprAST &node) = 0;
  virtual void visit(const IfExprAST &node) = 0;
  virtual void visit(const FunctionCallExprAST &node) = 0;
  virtual void visit(const FunctionPrototypeAST &node) = 0;
  virtual void visit(FunctionAST &node) = 0;
};

class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual void accept(ASTVisitor &visitor) const noexcept = 0;
};

class NumberExprAST : public ExprAST {
  double val;

public:
  NumberExprAST(double _val) noexcept : val(_val) {}
  void accept(ASTVisitor &visitor) const noexcept override;
  double getVal() const noexcept { return this->val; }
};

class VariableExprAST : public ExprAST {
  std::string name;

public:
  VariableExprAST(const std::string &_name) noexcept : name(_name) {}

  void accept(ASTVisitor &visitor) const noexcept override;
  std::string getName() const noexcept { return this->name; }
};

class BinaryExprAST : public ExprAST {
private:
  char op;

public:
  std::unique_ptr<ExprAST> Lhs, Rhs;
  BinaryExprAST(char _op, std::unique_ptr<ExprAST> _Lhs,
                std::unique_ptr<ExprAST> _Rhs) noexcept
      : op(_op), Lhs(std::move(_Lhs)), Rhs(std::move(_Rhs)) {}

  char getOp() const noexcept { return this->op; }
  void accept(ASTVisitor &visitor) const noexcept override;
};

class UnaryExprAST : public ExprAST {
private:
  char opcode;

public:
  std::unique_ptr<ExprAST> operand;

  UnaryExprAST(char _opcode, std::unique_ptr<ExprAST> _operand)
      : opcode(_opcode), operand(std::move(_operand)) {}

  char getOpcode() const noexcept { return this->opcode; }

  void accept(ASTVisitor &visitor) const noexcept override;
};

class IfExprAST : public ExprAST {
public:
  std::unique_ptr<ExprAST> cond, then, otherwise;

  IfExprAST(std::unique_ptr<ExprAST> _cond, std::unique_ptr<ExprAST> _then,
            std::unique_ptr<ExprAST> _otherwise) noexcept
      : cond(std::move(_cond)), then(std::move(_then)),
        otherwise(std::move(_otherwise)) {}

  void accept(ASTVisitor &visitor) const noexcept override;
};

class FunctionCallExprAST : public ExprAST {
private:
  std::string caller;

public:
  std::vector<std::unique_ptr<ExprAST>> args;

  FunctionCallExprAST(const std::string &_caller,
                      std::vector<std::unique_ptr<ExprAST>> _args) noexcept
      : caller(_caller), args(std::move(_args)) {}

  std::string getCaller() const noexcept { return this->caller; }
  void accept(ASTVisitor &visitor) const noexcept override;
};

// Represents a functions declaration
class FunctionPrototypeAST {
private:
  std::string name;
  std::vector<std::string> args;
  unsigned precedence;
  bool isOperator;

public:
  FunctionPrototypeAST(const std::string &_name, std::vector<std::string> _args,
                       bool _isOperator = false,
                       unsigned _precedence = 0) noexcept
      : name(_name), args(std::move(_args)), isOperator(_isOperator),
        precedence(_precedence) {};

  std::string getName() const noexcept { return name; }
  std::vector<std::string> getArgs() const noexcept { return args; }

  bool isUnaryOp() const noexcept {
    return this->isOperator && this->args.size() == 1;
  }
  bool isBinaryOp() const noexcept {
    return this->isOperator && this->args.size() == 2;
  }

  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return this->name[this->name.size() - 1];
  }

  unsigned getBinaryPrecedence() const { return this->precedence; }

  void accept(ASTVisitor &visitor) const noexcept;
};

class FunctionAST {
public:
  std::unique_ptr<FunctionPrototypeAST> prototype;
  std::unique_ptr<ExprAST> body;

  FunctionAST(std::unique_ptr<FunctionPrototypeAST> _prototype,
              std::unique_ptr<ExprAST> _body) noexcept
      : prototype(std::move(_prototype)), body(std::move(_body)) {}

  void accept(ASTVisitor &visitor) noexcept;
};

} // namespace monty
