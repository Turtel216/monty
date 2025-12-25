#pragma once

#include "../include/ast.hpp"
#include <memory>

namespace ggc {
class Parser {
public:
  Parser() noexcept : panicing(false) {}

  void handleTopLevelExpression() noexcept;

private:
  bool panicing;
  char curToken;
  std::string identifierStr;

  void getNextToken() noexcept;

  std::unique_ptr<ExprAST> parseExpresion() noexcept;
  std::unique_ptr<ExprAST> parseIdentifierExpr() noexcept;
  std::unique_ptr<ExprAST> parsePrimery() noexcept;
  std::unique_ptr<ExprAST> parseBinOpRhs() noexcept;
  std::unique_ptr<ExprAST> parseNumberExpr() noexcept;
  std::unique_ptr<ExprAST> parseParenExpr() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parsePrototype() noexcept;
  std::unique_ptr<FunctionAST> parseDefinition() noexcept;
  std::unique_ptr<FunctionPrototypeAST> parseExtern() noexcept;
  std::unique_ptr<FunctionAST> parseTopLevelExpr() noexcept;
};
} // namespace ggc
