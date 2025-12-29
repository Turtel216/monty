#include "../include/driver.hpp"

namespace monty {

void process(CodeGenerator &generator, Parser &parser) noexcept {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (parser.getCurrentToken()) {
    case token_eof:
      return;
    case ';': // ignore top-level semicolons.
      parser.getNextToken();
      break;
    case token_def:
      handleDefinition(generator, parser);
      break;
    case token_extern:
      handleExtern(generator, parser);
      break;
    default:
      handleTopLevelExpression(generator, parser);
      break;
    }
  }
}

void handleExtern(CodeGenerator &generator, Parser &parser) noexcept {
  if (auto protoAST = parser.parseExtern()) {
    generator.visit(*protoAST);
    if (auto *fnIR = generator.getLastFunctionValue()) {
      fprintf(stderr, "Read extern: ");
      fnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      generator.functionPrototypes[protoAST->getName()] = std::move(protoAST);
    }
  } else {
    // Skip token for error recovery.
    parser.getNextToken();
  }
}
void handleDefinition(CodeGenerator &generator, Parser &parser) noexcept {
  if (auto fnAST = parser.parseDefinition()) {
    generator.visit(*fnAST);

    if (auto *fnIR = generator.getLastFunctionValue()) {
      fprintf(stderr, "Read function definition:");
      fnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      generator.exitOnErr(generator.jit->addModule(llvm::orc::ThreadSafeModule(
          std::move(generator.llvmModule), std::move(generator.llvmContext))));

      generator.initializeModuleAndPassManager();
    }
  } else {
    // Skip token for error recovery.
    parser.getNextToken();
  }
}
void handleTopLevelExpression(CodeGenerator &generator,
                              Parser &parser) noexcept {
  // Evaluate a top-level expression into an anonymous function.
  if (auto fnAST = parser.parseTopLevelExpr()) {
    generator.visit(*fnAST);
    if (auto *fnIR = generator.getLastFunctionValue()) {
      // Create a ResourceTracker to track JIT'd memory allocated to our
      // anonymous expression -- that way we can free it after executing.
      auto RT = generator.jit->getMainJITDylib().createResourceTracker();

      auto TSM = llvm::orc::ThreadSafeModule(std::move(generator.llvmModule),
                                             std::move(generator.llvmContext));
      generator.exitOnErr(generator.jit->addModule(std::move(TSM), RT));
      generator.initializeModuleAndPassManager();

      // Search the JIT for the __anon_expr symbol.
      auto ExprSymbol =
          generator.exitOnErr(generator.jit->lookup("__anon_expr"));

      // Get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.
      double (*FP)() = ExprSymbol.getAddress().toPtr<double (*)()>();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // Delete the anonymous expression module from the JIT.
      generator.exitOnErr(RT->remove());
    }
  } else {
    // Skip token for error recovery.
    parser.getNextToken();
  }
}
} // namespace monty
