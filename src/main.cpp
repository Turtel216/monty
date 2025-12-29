#include "../include/cli.hpp"
#include "../include/driver.hpp"
#include "../include/generator.hpp"
#include "../include/parser.hpp"
#include <iostream>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

int main(int argc, char *argv[]) {

  try { // monty::Cli might throw exception
    monty::Cli cli(argc, argv);

    if (cli.help_requested) {
      cli.print_usage(argv[0]);
      return 0;
    }

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
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
