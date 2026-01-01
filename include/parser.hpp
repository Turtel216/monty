#pragma once

#include "../include/ast.hpp"
#include <istream>
#include <map>
#include <memory>

namespace monty {
namespace syn {

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

  token_let = -13,
  token_in = -14,
};

class Parser {
public:
  Parser(std::map<char, int> &_binopPrecedence,
         std::istream &_inputStream) noexcept
      : panicing(false), binopPrecedence(_binopPrecedence),
        inputStream(_inputStream) {}

  int getNextToken() noexcept;
  int getCurrentToken() const noexcept { return this->curToken; }

  std::unique_ptr<ast::FunctionAST> parseTopLevelExpr() noexcept;
  std::unique_ptr<ast::FunctionAST> parseDefinition() noexcept;
  std::unique_ptr<ast::FunctionPrototypeAST> parseExtern() noexcept;

private:
  bool panicing;
  char curToken;
  std::string identifierStr;
  double numVal;
  std::map<char, int> &binopPrecedence;
  std::istream &inputStream;

  // TODO add better compiler error handling
  std::unique_ptr<ast::ExprAST> logError(const char *Str) const noexcept;
  std::unique_ptr<ast::FunctionPrototypeAST>

  logErrorP(const char *Str) const noexcept;
  int getToken() noexcept;
  int getNextChar() noexcept { return this->inputStream.get(); }

  std::unique_ptr<ast::ExprAST> parseExpression() noexcept;
  std::unique_ptr<ast::ExprAST> parseIdentifierExpr() noexcept;
  std::unique_ptr<ast::ExprAST> parsePrimery() noexcept;
  std::unique_ptr<ast::ExprAST> parseUnary() noexcept;
  std::unique_ptr<ast::ExprAST>
  parseBinOpRhs(int exprPrec, std::unique_ptr<ast::ExprAST> Lhs) noexcept;
  std::unique_ptr<ast::ExprAST> parseIfExpr() noexcept;
  std::unique_ptr<ast::ExprAST> parseLetExpr() noexcept;
  std::unique_ptr<ast::ExprAST> parseNumberExpr() noexcept;
  std::unique_ptr<ast::ExprAST> parseParenExpr() noexcept;
  std::unique_ptr<ast::FunctionPrototypeAST> parsePrototype() noexcept;

  int getTokenPrecedence() const noexcept;
};
} // namespace syn
} // namespace monty
