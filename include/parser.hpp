#pragma once

#include "../include/ast.hpp"
#include <map>
#include <memory>

namespace ggc {

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

  void handleTopLevelExpression() noexcept;

private:
  bool panicing;
  char curToken;
  std::string identifierStr;
  double numVal;
  static std::map<char, int> binopPrecedence;

  int getNextToken() noexcept;
  int getToken() noexcept;

  // TODO add better compiler error handling
  std::unique_ptr<ExprAST> logError(const char *Str) const noexcept;
  std::unique_ptr<FunctionPrototypeAST>
  logErrorP(const char *Str) const noexcept;

  std::unique_ptr<ExprAST> parseExpresion() noexcept;
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
} // namespace ggc
