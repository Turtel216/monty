#include "../include/cli.hpp"
#include "../include/driver.hpp"
#include "../include/generator.hpp"
#include "../include/parser.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

int main(int argc, char *argv[]) {

  try { // monty::Cli might throw exception
    monty::Cli cli(argc, argv);

    if (cli.help_requested) {
      cli.print_usage(argv[0]);
      return 0;
    }

    // Needs to be accesed by both the generator and the parser
    std::map<char, int> binopPrecedence;
    binopPrecedence['='] = 2;
    binopPrecedence['<'] = 10;
    binopPrecedence['+'] = 20;
    binopPrecedence['-'] = 20;
    binopPrecedence['*'] = 40;

    monty::CodeGenerator generator{binopPrecedence};
    std::cout << "Here!\n";

    // Old Jit compiler code
    // llvm::InitializeNativeTarget();
    // llvm::InitializeNativeTargetAsmPrinter();
    // llvm::InitializeNativeTargetAsmParser();

    monty::Parser parser{binopPrecedence};
    fprintf(stderr, "ready> ");
    parser.getNextToken();

    monty::process(generator, parser);

    auto Filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
      llvm::errs() << "Could not open file: " << EC.message();
      return 1;
    }

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;

    if (generator.targetMachine->addPassesToEmitFile(pass, dest, nullptr,
                                                     fileType)) {
      llvm::errs() << "TargetMachine can't emit a file of this type";
      return 1;
    }

    pass.run(*generator.llvmModule);
    dest.flush();

    // generator.llvmModule->print(llvm::errs(), nullptr);

    llvm::outs() << "Wrote " << Filename << "\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
