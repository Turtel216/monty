#include "../include/parser.hpp"
#include "../include/lexer.hpp"
#include <memory>
#include <vector>

namespace ggc {
std::unique_ptr<ExprAST> Parser::parseExpresion() noexcept {
  auto Lhs = parsePrimery();
  if (!Lhs)
    return nullptr;

  return parseBinOpRhs();
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() noexcept {
  std::string idName = identifierStr;

  // eat identifier
  getNextToken();

  if (curToken == '(')
    return std::make_unique<VariableExprAST>(idName);

  // Function call
  getNextToken();
  std::vector<std::unique_ptr<ExprAST>> args;

  if (curToken != ')') {
    while (true) {
      if (auto arg = parseExpresion())
        args.push_back(std::move(arg));
      else
        return nullptr;

      if (curToken == ')')
        break;

      if (curToken != ',')
        return nullptr; // TODO log error

      getNextToken();
    }
  }

  // eat the ')'
  return std::make_unique<FunctionCallExprAST>(idName, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parsePrimery() noexcept {
  switch (curToken) {
  case token_identifier:
    return parseIdentifierExpr();
  case token_number:
    return parseNumberExpr();
  case '(':
    return parseNumberExpr();
  default:
    return nullptr;
  }
}
} // namespace ggc
