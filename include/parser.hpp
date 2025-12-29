#pragma once

#include "../include/ast.hpp"
#include <map>
#include <memory>

namespace monty {

enum Token {
  token_eof = -1,

  token_def = -2,
  token_extern = -3,

  token_identifier = -4,
  token_number = -5,

  token_if = -6,
  token_then = -7,
  token_else = -8,

  token_binary = -9,
  token_unary = -12,
};

class Parser {
public:
  Parser(std::map<char, int> &_binopPrecedence) noexcept
      : panicing(false), binopPrecedence(_binopPrecedence) {}

  int getNextToken() noexcept;
  int getCurrentToken() const noexcept { return this->curToken; }

  std::unique_ptr<FunctionAST> parseTopLevelExpr() noexcept;
  std::unique_ptr<FunctionAST> parseDefinition() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parseExtern() noexcept;

private:
  bool panicing;
  char curToken;
  std::string identifierStr;
  double numVal;
  std::map<char, int> &binopPrecedence;

  // TODO add better compiler error handling
  std::unique_ptr<ExprAST> logError(const char *Str) const noexcept;
  std::unique_ptr<FunctionPrototypeAST>

  logErrorP(const char *Str) const noexcept;
  int getToken() noexcept;

  std::unique_ptr<ExprAST> parseExpression() noexcept;
  std::unique_ptr<ExprAST> parseIdentifierExpr() noexcept;
  std::unique_ptr<ExprAST> parsePrimery() noexcept;
  std::unique_ptr<ExprAST> parseBinOpRhs(int exprPrec,
                                         std::unique_ptr<ExprAST> Lhs) noexcept;
  std::unique_ptr<ExprAST> ParseIfExpr() noexcept;
  std::unique_ptr<ExprAST> parseNumberExpr() noexcept;
  std::unique_ptr<ExprAST> parseParenExpr() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parsePrototype() noexcept;

  int getTokenPrecedence() const noexcept;
};
} // namespace monty
