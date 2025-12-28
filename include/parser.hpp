#pragma once

#include "../include/ast.hpp"
#include "generator.hpp"
#include <map>
#include <memory>

namespace monty {

enum Token {
  token_eof = -1,

  token_def = -2,
  token_extern = -3,

  token_identifier = -4,
  token_number = -5,
};

class Parser {
public:
  Parser() noexcept : panicing(false) {
    // Initialize standard binary operators.
    binopPrecedence['<'] = 10;
    binopPrecedence['+'] = 20;
    binopPrecedence['-'] = 20;
    binopPrecedence['*'] = 40;
  }

  void replLoop(
      CodeGenerator &generator) noexcept; // TODO move logic to compiler driver
  int getNextToken() noexcept;

private:
  bool panicing;
  char curToken;
  std::string identifierStr;
  double numVal;
  static inline std::map<char, int> binopPrecedence;

  // TODO add better compiler error handling
  std::unique_ptr<ExprAST> logError(const char *Str) const noexcept;
  std::unique_ptr<FunctionPrototypeAST>

  logErrorP(const char *Str) const noexcept;
  int getToken() noexcept;

  void handleExtern(CodeGenerator &generator) noexcept;
  void handleDefinition(CodeGenerator &generator) noexcept;
  void handleTopLevelExpression(CodeGenerator &generator) noexcept;

  std::unique_ptr<ExprAST> parseExpression() noexcept;
  std::unique_ptr<ExprAST> parseIdentifierExpr() noexcept;
  std::unique_ptr<ExprAST> parsePrimery() noexcept;
  std::unique_ptr<ExprAST> parseBinOpRhs(int exprPrec,
                                         std::unique_ptr<ExprAST> Lhs) noexcept;
  std::unique_ptr<ExprAST> parseNumberExpr() noexcept;
  std::unique_ptr<ExprAST> parseParenExpr() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parsePrototype() noexcept;
  std::unique_ptr<FunctionAST> parseDefinition() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parseExtern() noexcept;
  std::unique_ptr<FunctionAST> parseTopLevelExpr() noexcept;

  int getTokenPrecedence() const noexcept;
};
} // namespace monty
