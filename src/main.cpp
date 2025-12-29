#include "../include/driver.hpp"
#include "../include/generator.hpp"
#include "../include/parser.hpp"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

int main(int argc, char *argv[]) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  monty::Parser parser;
  monty::CodeGenerator generator;
  fprintf(stderr, "ready> ");
  parser.getNextToken();

  monty::process(generator, parser);

  generator.llvmModule->print(llvm::errs(), nullptr);

  return 0;
}
