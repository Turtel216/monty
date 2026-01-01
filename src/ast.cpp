#include "../include/ast.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace monty {
namespace ast {
void NumberExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void VariableExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void BinaryExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void UnaryExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void IfExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void LetExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void FunctionCallExprAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void FunctionPrototypeAST::accept(ASTVisitor &visitor) const noexcept {
  visitor.visit(*this);
}

void FunctionAST::accept(ASTVisitor &visitor) noexcept { visitor.visit(*this); }
} // namespace ast
} // namespace monty
