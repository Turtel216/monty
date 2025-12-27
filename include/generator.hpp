#pragma once

#include "ast.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <map>
#include <memory>
#include <string>

namespace monty {

struct CodeGenerator : public ASTVisitor {
  std::unique_ptr<llvm::LLVMContext> llvmContext;
  std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
  std::unique_ptr<llvm::Module> llvmModule;
  std::map<std::string, llvm::Value *> namedValues;
  llvm::Value *lastValue;

  // TODO add proper constructors

  llvm::Value *logError(const char *str) const noexcept;

  void visit(const NumberExprAST &node) override;
  void visit(const VariableExprAST &node) override;
  void visit(const BinaryExprAST &node) override;
  void visit(const FunctionCallExprAST &node) override;
  void visit(const FunctionPrototypeAST &node) override;
  void visit(const FunctionAST &node) override;
};
} // namespace monty
