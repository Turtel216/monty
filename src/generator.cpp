#include "../include/generator.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <memory>
namespace monty {
void CodeGenerator::initializeModuleAndPassManager() noexcept {
  // Create context and module
  this->llvmContext = std::make_unique<llvm::LLVMContext>();
  this->llvmModule =
      std::make_unique<llvm::Module>("Monty JIT", *this->llvmContext);
  this->llvmModule->setDataLayout(jit->getDataLayout());

  // Create IRBuilder
  this->llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*this->llvmContext);

  // Create pass and analysis managers
  this->fpm = std::make_unique<llvm::FunctionPassManager>();
  this->lam = std::make_unique<llvm::LoopAnalysisManager>();
  this->fam = std::make_unique<llvm::FunctionAnalysisManager>();
  this->cgam = std::make_unique<llvm::CGSCCAnalysisManager>();
  this->mam = std::make_unique<llvm::ModuleAnalysisManager>();
  this->pic = std::make_unique<llvm::PassInstrumentationCallbacks>();
  this->si =
      std::make_unique<llvm::StandardInstrumentations>(*this->llvmContext,
                                                       /*DebugLogging*/ true);
  this->si->registerCallbacks(*this->pic, this->mam.get());

  // Add transform passes.
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  this->fpm->addPass(llvm::InstCombinePass());
  // Reassociate expressions.
  this->fpm->addPass(llvm::ReassociatePass());
  // Eliminate Common SubExpressions.
  this->fpm->addPass(llvm::GVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  this->fpm->addPass(llvm::SimplifyCFGPass());

  // Register analysis passes used in these transform passes.
  llvm::PassBuilder pb;
  pb.registerModuleAnalyses(*mam);
  pb.registerFunctionAnalyses(*fam);
  pb.crossRegisterProxies(*this->lam, *this->fam, *this->cgam, *this->mam);
}

void CodeGenerator::visit(const NumberExprAST &node) {
  this->lastValue =
      llvm::ConstantFP::get(*llvmContext, llvm::APFloat(node.getVal()));
}

void CodeGenerator::visit(const VariableExprAST &node) {
  llvm::Value *v = namedValues[node.getName()];

  if (!v) {
    this->lastValue = logError("Unkown variable name");
    return;
  }

  this->lastValue = v;
}

void CodeGenerator::visit(const BinaryExprAST &node) {
  node.Lhs->accept(*this);
  llvm::Value *l = this->lastValue;
  node.Rhs->accept(*this);
  llvm::Value *r = this->lastValue;

  if (!l || !r) {
    this->lastValue = nullptr;
    return;
  }

  switch (node.getOp()) {
  case '+':
    this->lastValue = this->llvmBuilder->CreateFAdd(l, r, "addtmp");
    return;
  case '-':
    this->lastValue = this->llvmBuilder->CreateFSub(l, r, "subtmp");
    return;
  case '*':
    this->lastValue = this->llvmBuilder->CreateFMul(l, r, "multmp");
    return;
  case '<':
    l = this->llvmBuilder->CreateFCmpULT(l, r, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    this->lastValue = this->llvmBuilder->CreateUIToFP(
        l, llvm::Type::getDoubleTy(*this->llvmContext), "booltmp");
    return;
  default:
    break;
  }

  llvm::Function *F = getFunction(std::string("binary") + node.getOp());
  assert(F && "binary operator not found!");

  llvm::Value *Ops[2] = {l, r};
  this->lastValue = this->llvmBuilder->CreateCall(F, Ops, "binop");
}

void CodeGenerator::visit(const UnaryExprAST &node) {
  node.operand->accept(*this);
  llvm::Value *OperandV = lastValue;
  if (!OperandV) {
    lastValue = nullptr;
    return;
  }

  llvm::Function *F = getFunction(std::string("unary") + node.getOpcode());
  if (!F) {
    lastValue = logError("Unknown unary operator");
    return;
  }

  lastValue = this->llvmBuilder->CreateCall(F, OperandV, "unop");
}

void CodeGenerator::visit(const IfExprAST &node) {
  // Emit expression for the condition
  node.cond->accept(*this);
  llvm::Value *condV = this->lastValue;

  if (!condV) {
    this->lastValue = nullptr;
    return;
  }

  // Compare the value to zero to get a truth value as 1-bit
  condV = this->llvmBuilder->CreateFCmpONE(
      condV, llvm::ConstantFP::get(*this->llvmContext, llvm::APFloat(0.0)),
      "ifcond");

  llvm::Function *function = this->llvmBuilder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(*this->llvmContext, "then", function);
  llvm::BasicBlock *elseBB =
      llvm::BasicBlock::Create(*this->llvmContext, "else");
  llvm::BasicBlock *mergeBB =
      llvm::BasicBlock::Create(*this->llvmContext, "ifcont");

  this->llvmBuilder->CreateCondBr(condV, thenBB, elseBB);

  // Emit then value.
  this->llvmBuilder->SetInsertPoint(thenBB);

  node.then->accept(*this);
  llvm::Value *thenV = lastValue;

  if (!thenV) {
    lastValue = nullptr;
    return;
  }

  this->llvmBuilder->CreateBr(mergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  thenBB = this->llvmBuilder->GetInsertBlock();

  // Emit else block.
  function->insert(function->end(), elseBB);
  this->llvmBuilder->SetInsertPoint(elseBB);

  node.otherwise->accept(*this);
  llvm::Value *elseV = this->lastValue;

  if (!elseV) {
    lastValue = nullptr;
    return;
  }

  this->llvmBuilder->CreateBr(mergeBB);
  // codegen of 'Else' can change the current block, update ElseBB for the PHI.
  elseBB = this->llvmBuilder->GetInsertBlock();

  // Emit merge block.
  function->insert(function->end(), mergeBB);
  this->llvmBuilder->SetInsertPoint(mergeBB);
  llvm::PHINode *pn = this->llvmBuilder->CreatePHI(
      llvm::Type::getDoubleTy(*this->llvmContext), 2, "iftmp");

  pn->addIncoming(thenV, thenBB);
  pn->addIncoming(elseV, elseBB);
  lastValue = pn;
}

void CodeGenerator::visit(const FunctionCallExprAST &node) {
  llvm::Function *calleeF = getFunction(node.getCaller());

  if (!calleeF) {
    this->lastValue = logError("Unknown function referenced");
    return;
  }

  // If argument mismatch error.
  if (calleeF->arg_size() != node.args.size()) {
    this->lastValue = logError("Incorrect # arguments passed");
    return;
  }

  std::vector<llvm::Value *> argsV;
  for (unsigned i = 0, e = node.args.size(); i != e; ++i) {
    node.args[i]->accept(*this);
    argsV.push_back(this->lastValue);

    if (!argsV.back()) {
      this->lastValue = nullptr;
      return;
    }
  }

  this->lastValue = this->llvmBuilder->CreateCall(calleeF, argsV, "calltmp");
}

void CodeGenerator::visit(const FunctionPrototypeAST &node) {
  std::vector<llvm::Type *> doubles(
      node.getArgs().size(), llvm::Type::getDoubleTy(*this->llvmContext));

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*this->llvmContext), doubles, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                             node.getName(), this->llvmModule.get());

  unsigned idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(node.getArgs()[idx++]);

  this->lastFunctionValue = F;
}

void CodeGenerator::visit(FunctionAST &node) {
  auto &P = *node.prototype;
  this->functionPrototypes[node.prototype->getName()] =
      std::move(node.prototype);

  llvm::Function *function = getFunction(P.getName());
  if (!function) {
    lastFunctionValue = nullptr;
    return;
  }

  // If this is an operator, install it.
  if (P.isBinaryOp())
    binopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*this->llvmContext, "entry", function);
  this->llvmBuilder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  this->namedValues.clear();
  for (auto &Arg : function->args())
    this->namedValues[std::string(Arg.getName())] = &Arg;

  node.body->accept(*this);
  if (llvm::Value *retVal = this->lastValue) {
    // Finish off the function.
    this->llvmBuilder->CreateRet(retVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*function);

    this->fpm->run(*function, *this->fam);

    this->lastFunctionValue = function;
    return;
  }

  // Error reading body, remove function.
  function->eraseFromParent();
  this->lastFunctionValue = nullptr;
}

llvm::Value *CodeGenerator::logError(const char *str) const noexcept {
  fprintf(stderr, "Error: %s\n", str);
  return nullptr;
}

llvm::Function *CodeGenerator::getFunction(std::string name) noexcept {
  // check if function has already bin added to the current module
  if (auto *f = this->llvmModule->getFunction(name)) {
    return f;
  }

  // check if the declaration can be generated from an existing prototype
  auto fi = this->functionPrototypes.find(name);
  if (fi != this->functionPrototypes.end()) {
    fi->second->accept(*this);
    return lastFunctionValue;
  }

  return nullptr;
}
} // namespace monty
