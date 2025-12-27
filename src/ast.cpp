#include "../include/ast.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace monty {
llvm::Value *NumberExprAST::codegen(GeneratorContext &context) const noexcept {
  return llvm::ConstantFP::get(*context.llvmContext, llvm::APFloat(this->val));
}

llvm::Value *
VariableExprAST::codegen(GeneratorContext &context) const noexcept {
  llvm::Value *v = context.namedValues[this->name];

  if (!v)
    return context.logError("Unkown variable name");

  return v;
}

llvm::Value *BinaryExprAST::codegen(GeneratorContext &context) const noexcept {
  llvm::Value *l = Lhs->codegen(context);
  llvm::Value *r = Rhs->codegen(context);
  if (!l || !r)
    return nullptr;

  switch (this->op) {
  case '+':
    return context.llvmBuilder->CreateFAdd(l, r, "addtmp");
  case '-':
    return context.llvmBuilder->CreateFSub(l, r, "subtmp");
  case '*':
    return context.llvmBuilder->CreateFMul(l, r, "multmp");
  case '<':
    l = context.llvmBuilder->CreateFCmpULT(l, r, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return context.llvmBuilder->CreateUIToFP(
        l, llvm::Type::getDoubleTy(*context.llvmContext), "booltmp");
  default:
    return context.logError("invalid binary operator");
  }
}

llvm::Value *
FunctionCallExprAST::codegen(GeneratorContext &context) const noexcept {
  llvm::Function *calleeF = context.llvmModule->getFunction(this->caller);

  if (!calleeF)
    return context.logError("Unknown function referenced");

  // If argument mismatch error.
  if (calleeF->arg_size() != this->args.size())
    return context.logError("Incorrect # arguments passed");

  std::vector<llvm::Value *> argsV;
  for (unsigned i = 0, e = args.size(); i != e; ++i) {
    argsV.push_back(args[i]->codegen(context));
    if (!argsV.back())
      return nullptr;
  }

  return context.llvmBuilder->CreateCall(calleeF, argsV, "calltmp");
}

llvm::Function *
FunctionPrototypeAST::codegen(GeneratorContext &context) const noexcept {
  std::vector<llvm::Type *> doubles(
      args.size(), llvm::Type::getDoubleTy(*context.llvmContext));

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*context.llvmContext), doubles, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage, this->name,
                             context.llvmModule.get());

  unsigned idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(this->args[idx++]);

  return F;
}

llvm::Function *FunctionAST::codegen(GeneratorContext &context) const noexcept {
  llvm::Function *function =
      context.llvmModule->getFunction(this->prototype->getName());

  if (!function)
    function = this->prototype->codegen(context);

  if (!function)
    return nullptr;

  if (!function->empty())
    return (llvm::Function *)context.logError("Function cannot be redefined.");

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*context.llvmContext, "entry", function);
  context.llvmBuilder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  context.namedValues.clear();
  for (auto &Arg : function->args())
    context.namedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = body->codegen(context)) {
    // Finish off the function.
    context.llvmBuilder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*function);

    return function;
  }

  // Error reading body, remove function.
  function->eraseFromParent();
  return nullptr;
}
} // namespace monty
