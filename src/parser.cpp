#include "../include/parser.hpp"
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
        return logError("Expected ')' or ',' in argument list");

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
    return logError("unknown token when expecting an expression");
  }
}

std::unique_ptr<ExprAST> Parser::logError(const char *Str) const noexcept {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<FunctionPrototypeAST>
Parser::logErrorP(const char *Str) const noexcept {
  logError(Str);
  return nullptr;
}

int Parser::getNextToken() noexcept { return curToken = getToken(); }

int Parser::getToken() noexcept {
  static int lastChar = ' ';

  // Skip any whitespace.
  while (isspace(lastChar))
    lastChar = getchar();

  if (isalpha(lastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    identifierStr = lastChar;
    while (isalnum((lastChar = getchar())))
      identifierStr += lastChar;

    if (identifierStr == "def")
      return token_def;
    if (identifierStr == "extern")
      return token_extern;
    return token_identifier;
  }

  if (isdigit(lastChar) || lastChar == '.') { // Number: [0-9.]+
    std::string numStr;
    do {
      numStr += lastChar;
      lastChar = getchar();
    } while (isdigit(lastChar) || lastChar == '.');

    numVal = strtod(numStr.c_str(), nullptr);
    return token_number;
  }

  if (lastChar == '#') {
    // Comment until end of line.
    do
      lastChar = getchar();
    while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

    if (lastChar != EOF)
      return getToken();
  }

  // Check for end of file.  Don't eat the EOF.
  if (lastChar == EOF)
    return token_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = lastChar;
  lastChar = getchar();
  return ThisChar;
}
} // namespace ggc
