#include "../include/generator.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

namespace monty {

CodeGenerator::CodeGenerator() noexcept {

  this->llvmContext = std::make_unique<llvm::LLVMContext>();
  this->llvmModule =
      std::make_unique<llvm::Module>("Monty JIT", *this->llvmContext);

  this->llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*this->llvmContext);
}

void CodeGenerator::visit(const NumberExprAST &node) {
  lastValue = llvm::ConstantFP::get(*llvmContext, llvm::APFloat(node.getVal()));
}

void CodeGenerator::visit(const VariableExprAST &node) {
  llvm::Value *v = namedValues[node.getName()];

  if (!v) {
    lastValue = logError("Unkown variable name");
    return;
  }

  lastValue = v;
}

void CodeGenerator::visit(const BinaryExprAST &node) {
  node.Lhs->accept(*this);
  llvm::Value *l = lastValue;
  node.Rhs->accept(*this);
  llvm::Value *r = lastValue;

  if (!l || !r) {
    lastValue = nullptr;
    return;
  }

  switch (node.op) {
  case '+':
    lastValue = this->llvmBuilder->CreateFAdd(l, r, "addtmp");
    break;
  case '-':
    lastValue = this->llvmBuilder->CreateFSub(l, r, "subtmp");
    break;
  case '*':
    lastValue = this->llvmBuilder->CreateFMul(l, r, "multmp");
    break;
  case '<':
    l = this->llvmBuilder->CreateFCmpULT(l, r, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    lastValue = this->llvmBuilder->CreateUIToFP(
        l, llvm::Type::getDoubleTy(*this->llvmContext), "booltmp");
    break;
  default:
    lastValue = logError("invalid binary operator");
    break;
  }
}

void CodeGenerator::visit(const FunctionCallExprAST &visitor) {
  llvm::Function *calleeF = this->llvmModule->getFunction(visitor.caller);

  if (!calleeF) {
    lastValue = logError("Unknown function referenced");
    return;
  }

  // If argument mismatch error.
  if (calleeF->arg_size() != visitor.args.size()) {
    lastValue = logError("Incorrect # arguments passed");
    return;
  }

  std::vector<llvm::Value *> argsV;
  for (unsigned i = 0, e = visitor.args.size(); i != e; ++i) {
    visitor.args[i]->accept(*this);
    argsV.push_back(lastValue);

    if (!argsV.back()) {
      lastValue = nullptr;
      return;
    }
  }

  lastValue = this->llvmBuilder->CreateCall(calleeF, argsV, "calltmp");
}

void CodeGenerator::visit(const FunctionPrototypeAST &node) {
  std::vector<llvm::Type *> doubles(
      node.args.size(), llvm::Type::getDoubleTy(*this->llvmContext));

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*this->llvmContext), doubles, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, node.name, this->llvmModule.get());

  unsigned idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(node.args[idx++]);

  lastFunctionValue = F;
}

void CodeGenerator::visit(const FunctionAST &node) {
  llvm::Function *function =
      this->llvmModule->getFunction(node.prototype->getName());

  if (!function) {
    node.prototype->accept(*this);
    function = lastFunctionValue;
  }

  if (!function) {
    lastValue = nullptr;
    return;
  }

  if (!function->empty()) {
    lastFunctionValue =
        (llvm::Function *)logError("Function cannot be redefined.");
    return;
  }

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*this->llvmContext, "entry", function);
  this->llvmBuilder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  this->namedValues.clear();
  for (auto &Arg : function->args())
    this->namedValues[std::string(Arg.getName())] = &Arg;

  node.body->accept(*this);
  if (llvm::Value *RetVal = lastValue) {
    // Finish off the function.
    this->llvmBuilder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*function);

    lastFunctionValue = function;
    return;
  }

  // Error reading body, remove function.
  function->eraseFromParent();
  lastFunctionValue = nullptr;
}

llvm::Value *CodeGenerator::logError(const char *str) const noexcept {
  fprintf(stderr, "Error: %s\n", str);
  return nullptr;
}
} // namespace monty
