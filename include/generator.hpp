#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <map>
#include <memory>
#include <string>

namespace monty {

struct GeneratorContext {
  std::unique_ptr<llvm::LLVMContext> llvmContext;
  std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
  std::unique_ptr<llvm::Module> llvmModule;
  std::map<std::string, llvm::Value *> namedValues;

  llvm::Value *logError(const char *str) const noexcept;
};
} // namespace monty
