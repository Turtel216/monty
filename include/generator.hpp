#pragma once

#include "ast.hpp"
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassInstrumentation.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <map>
#include <memory>
#include <string>

namespace monty {
namespace gen {

class CodeGenerator : public ast::ASTVisitor {
private:
  llvm::Value *lastValue;
  llvm::Function *lastFunctionValue;
  // LLVM analysis managers
  /*
  std::unique_ptr<llvm::FunctionPassManager> fpm;
  std::unique_ptr<llvm::LoopAnalysisManager> lam;
  std::unique_ptr<llvm::FunctionAnalysisManager> fam;
  std::unique_ptr<llvm::CGSCCAnalysisManager> cgam;
  std::unique_ptr<llvm::ModuleAnalysisManager> mam;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> pic;
  std::unique_ptr<llvm::StandardInstrumentations> si;
  */

  std::map<char, int> &binopPrecedence;

  llvm::Function *getFunction(std::string name) noexcept;
  llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function,
                                           llvm::StringRef varName);

public:
  // LLVM builder utils
  std::unique_ptr<llvm::LLVMContext> llvmContext;
  std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
  std::unique_ptr<llvm::Module> llvmModule;
  llvm::TargetMachine *targetMachine;
  llvm::Triple targetTriplet;
  // Symbol table
  std::map<std::string, llvm::AllocaInst *> namedValues;
  std::map<std::string, std::unique_ptr<ast::FunctionPrototypeAST>>
      functionPrototypes;

  // LLVM util for exiting on code generation error
  llvm::ExitOnError exitOnErr;

  CodeGenerator(std::map<char, int> &_binopPrecedence) noexcept;

  // TODO: Update error handling
  llvm::Value *logError(const char *str) const noexcept;

  llvm::Value *getLastValue() const noexcept { return this->lastValue; }
  llvm::Function *getLastFunctionValue() const noexcept {
    return this->lastFunctionValue;
  }

  // ASTVisitor interface
  void visit(const ast::NumberExprAST &node) override;
  void visit(const ast::VariableExprAST &node) override;
  void visit(const ast::BinaryExprAST &node) override;
  void visit(const ast::UnaryExprAST &node) override;
  void visit(const ast::IfExprAST &node) override;
  void visit(const ast::LetExprAST &node) override;
  void visit(const ast::FunctionCallExprAST &node) override;
  void visit(const ast::FunctionPrototypeAST &node) override;
  void visit(ast::FunctionAST &node) override;
};
} // namespace gen
} // namespace monty
