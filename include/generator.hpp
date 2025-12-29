#pragma once

#include "../include/jit.hpp"
#include "ast.hpp"
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassInstrumentation.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <map>
#include <memory>
#include <string>

namespace monty {

class CodeGenerator : public ASTVisitor {
private:
  llvm::Value *lastValue;
  llvm::Function *lastFunctionValue;
  // LLVM analysis managers
  std::unique_ptr<llvm::FunctionPassManager> fpm;
  std::unique_ptr<llvm::LoopAnalysisManager> lam;
  std::unique_ptr<llvm::FunctionAnalysisManager> fam;
  std::unique_ptr<llvm::CGSCCAnalysisManager> cgam;
  std::unique_ptr<llvm::ModuleAnalysisManager> mam;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> pic;
  std::unique_ptr<llvm::StandardInstrumentations> si;

public:
  // LLVM builder utils
  std::unique_ptr<llvm::LLVMContext> llvmContext;
  std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
  std::unique_ptr<llvm::Module> llvmModule;
  // Symbol table
  std::map<std::string, llvm::Value *> namedValues;
  // JIT Compilation runtime
  std::unique_ptr<llvm::orc::KaleidoscopeJIT> jit;

  // LLVM util for exiting on code generation error
  llvm::ExitOnError exitOnErr;

  CodeGenerator() noexcept;

  // TODO: Update error handling
  llvm::Value *logError(const char *str) const noexcept;

  llvm::Value *getLastValue() const noexcept { return this->lastValue; }
  llvm::Function *getLastFunctionValue() const noexcept {
    return this->lastFunctionValue;
  }

  void initializeModuleAndPassManager() noexcept;

  // ASTVisitor interface
  void visit(const NumberExprAST &node) override;
  void visit(const VariableExprAST &node) override;
  void visit(const BinaryExprAST &node) override;
  void visit(const FunctionCallExprAST &node) override;
  void visit(const FunctionPrototypeAST &node) override;
  void visit(const FunctionAST &node) override;
};
} // namespace monty
