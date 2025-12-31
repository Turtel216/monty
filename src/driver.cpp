#include "../include/driver.hpp"
#include <cstdlib>

namespace monty {

void linkToRuntime(const std::string &output) {
  std::string command = "clang++ cpp-runtime/entry.cpp output.o -o " + output;
  std::system(command.c_str());
}

void cleanUp(const std::string &objectFile) {
  std::string command = "rm " + objectFile;
  std::system(command.c_str());
}

void process(CodeGenerator &generator, Parser &parser) noexcept {
  while (true) {
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
  } else {
    // Skip token for error recovery.
    parser.getNextToken();
  }
}
} // namespace monty
