#include "../include/driver.hpp"
#include <cstdlib>

namespace monty {

namespace drv {
void linkToRuntime(const std::string &output) {
  std::string command = "clang++ cpp-runtime/entry.cpp output.o -o " + output;
  std::system(command.c_str());
}

void cleanUp(const std::string &objectFile) {
  std::string command = "rm " + objectFile;
  std::system(command.c_str());
}

void process(gen::CodeGenerator &generator, syn::Parser &parser) noexcept {
  while (true) {
    switch (parser.getCurrentToken()) {
    case syn::token_eof:
      return;
    case ';': // ignore top-level semicolons.
      parser.getNextToken();
      break;
    case syn::token_def:
      handleDefinition(generator, parser);
      break;
    case syn::token_extern:
      handleExtern(generator, parser);
      break;
    default:
      handleTopLevelExpression(generator, parser);
      break;
    }
  }
}

void handleExtern(gen::CodeGenerator &generator, syn::Parser &parser) noexcept {
  if (auto protoAST = parser.parseExtern()) {
    generator.visit(*protoAST);
    if (auto *fnIR = generator.getLastFunctionValue()) {
      fprintf(stderr, "Read extern: ");
      fnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      generator.functionPrototypes[protoAST->getName()] = std::move(protoAST);
    }
  } else {
    // Error encountered, synchronize for recovery
    parser.synchronize();
  }
}
void handleDefinition(gen::CodeGenerator &generator,
                      syn::Parser &parser) noexcept {
  if (auto fnAST = parser.parseDefinition()) {
    generator.visit(*fnAST);

    if (auto *fnIR = generator.getLastFunctionValue()) {
      fprintf(stderr, "Read function definition:");
      fnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Error encountered, synchronize for recovery
    parser.synchronize();
  }
}
void handleTopLevelExpression(gen::CodeGenerator &generator,
                              syn::Parser &parser) noexcept {
  // Evaluate a top-level expression into an anonymous function.
  if (auto fnAST = parser.parseTopLevelExpr()) {
    generator.visit(*fnAST);
  } else {
    // Error encountered, synchronize for recovery
    parser.synchronize();
  }
}
} // namespace drv
} // namespace monty
