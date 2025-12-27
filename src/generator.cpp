#include "../include/generator.hpp"
#include <llvm/IR/Module.h>

namespace monty {
void CodeGenerator::visit(const NumberExprAST &visitor) {
  lastValue =
      llvm::ConstantFP::get(*llvmContext, llvm::APFloat(visitor.getVal()));
}

void CodeGenerator::visit(const VariableExprAST &visitor) {
  llvm::Value *v = namedValues[visitor.getName()];

  if (!v) {
    lastValue = logError("Unkown variable name");
    return;
  }

  lastValue = v;
}

void CodeGenerator::visit(const BinaryExprAST &visitor) {
  visitor.Lhs->accept(*this);
  llvm::Value *l = lastValue;
  visitor.Rhs->accept(*this);
  llvm::Value *r = lastValue;

  if (!l || !r) {
    lastValue = nullptr;
    return;
  }

  switch (visitor.op) {
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
} // namespace monty

// TODO:
/*
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
*/
