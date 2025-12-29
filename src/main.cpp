#include "../include/driver.hpp"
#include "../include/generator.hpp"
#include "../include/parser.hpp"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

int main(int argc, char *argv[]) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Needs to be accesed by both the generator and the parser
  std::map<char, int> binopPrecedence;
  binopPrecedence['<'] = 10;
  binopPrecedence['+'] = 20;
  binopPrecedence['-'] = 20;
  binopPrecedence['*'] = 40;

  monty::Parser parser{binopPrecedence};
  monty::CodeGenerator generator{binopPrecedence};
  fprintf(stderr, "ready> ");
  parser.getNextToken();

  monty::process(generator, parser);

  generator.llvmModule->print(llvm::errs(), nullptr);

  return 0;
}
