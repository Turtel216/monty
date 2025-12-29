#include "../include/parser.hpp"
#include <cctype>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <memory>
#include <vector>

namespace monty {
std::unique_ptr<ExprAST> Parser::parseExpression() noexcept {
  auto Lhs = parsePrimery();
  if (!Lhs)
    return nullptr;

  return parseBinOpRhs(0, std::move(Lhs));
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() noexcept {
  std::string idName = identifierStr;

  // eat identifier
  getNextToken();

  if (curToken != '(')
    return std::make_unique<VariableExprAST>(idName);

  // Function call
  getNextToken();
  std::vector<std::unique_ptr<ExprAST>> args;

  if (curToken != ')') {
    while (true) {
      if (auto arg = parseExpression())
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

std::unique_ptr<ExprAST>
Parser::parseBinOpRhs(int exprPrec, std::unique_ptr<ExprAST> Lhs) noexcept {
  // find precedence of bin op
  while (true) {
    int tokenPrec = getTokenPrecedence();

    if (tokenPrec < exprPrec)
      return Lhs;

    // is a binary operation
    int binOp = curToken;
    getNextToken(); // eat bin op

    auto Rhs = parsePrimery();
    if (!Rhs)
      return nullptr;

    int nextPrec = getTokenPrecedence();
    if (tokenPrec < nextPrec) {
      Rhs = parseBinOpRhs(tokenPrec + 1, std::move(Rhs));
      if (!Rhs)
        return nullptr;
    }

    // Combine Lhs and Rhs.
    Lhs =
        std::make_unique<BinaryExprAST>(binOp, std::move(Lhs), std::move(Rhs));
  }
}

std::unique_ptr<ExprAST> Parser::ParseIfExpr() noexcept {
  getNextToken(); // eat the if.

  // condition.
  auto cond = parseExpression();
  if (!cond)
    return nullptr;

  if (this->curToken != token_then)
    return logError("expected then");
  getNextToken(); // eat the then

  auto then = parseExpression();
  if (!then)
    return nullptr;

  if (this->curToken != token_else)
    return logError("expected else");

  getNextToken();

  auto otherwise = parseExpression();
  if (!otherwise)
    return nullptr;

  return std::make_unique<IfExprAST>(std::move(cond), std::move(then),
                                     std::move(otherwise));
}

std::unique_ptr<ExprAST> Parser::parsePrimery() noexcept {
  switch (curToken) {
  case token_identifier:
    return parseIdentifierExpr();
  case token_number:
    return parseNumberExpr();
  case '(':
    return parseParenExpr();
  case token_if:
    return ParseIfExpr();
  default:
    return logError("unknown token when expecting an expression");
  }
}

std::unique_ptr<ExprAST> Parser::parseNumberExpr() noexcept {
  auto result = std::make_unique<NumberExprAST>(numVal);
  getNextToken(); // consume the number
  return result;
}

std::unique_ptr<FunctionPrototypeAST> Parser::parsePrototype() noexcept {
  if (curToken != token_identifier)
    return logErrorP("Expected function name prototype");

  std::string fnName = identifierStr;
  getNextToken();

  if (curToken != '(')
    return logErrorP("Expected ')' in protortpe");

  // parse arguments
  std::vector<std::string> argNames;
  while (getNextToken() == token_identifier)
    argNames.push_back(identifierStr);

  if (curToken != ')')
    return logErrorP("Expected ')' in protortpe");

  // eat ')'
  getNextToken();

  return std::make_unique<FunctionPrototypeAST>(fnName, std::move(argNames));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition() noexcept {
  getNextToken(); // eat def.
  auto Proto = parsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = parseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
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

std::unique_ptr<FunctionPrototypeAST> Parser::parseExtern() noexcept {
  getNextToken(); // eat extern.
  return parsePrototype();
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr() noexcept {
  if (auto expr = parseExpression()) {
    // Make an anonymous proto.
    auto proto = std::make_unique<FunctionPrototypeAST>(
        "__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
  }
  return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseParenExpr() noexcept {
  getNextToken(); // eat (.
  auto V = parseExpression();
  if (!V)
    return nullptr;

  if (curToken != ')')
    return logError("expected ')'");

  getNextToken(); // eat ).
  return V;
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

    if (identifierStr == "fn")
      return token_def;
    if (identifierStr == "using")
      return token_extern;
    if (identifierStr == "if")
      return token_if;
    if (identifierStr == "then")
      return token_then;
    if (identifierStr == "else")
      return token_else;
    if (identifierStr == "binary")
      return token_binary;
    if (identifierStr == "unary")
      return token_unary;

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

int Parser::getTokenPrecedence() const noexcept {
  if (!isascii(curToken))
    return -1; // return invalid token code

  int tokenPrec = binopPrecedence[curToken];
  if (tokenPrec <= 0) // Invalid token
    return -1;

  return tokenPrec;
}

} // namespace monty
