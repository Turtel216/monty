#include "../include/parser.hpp"
#include <llvm/Support/raw_ostream.h>

int main(int argc, char *argv[]) {

  monty::Parser parser;
  monty::CodeGenerator generator;
  fprintf(stderr, "ready> ");
  parser.getNextToken();

  parser.replLoop(generator);

  generator.llvmModule->print(llvm::errs(), nullptr);

  return 0;
}
