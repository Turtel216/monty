#include "../include/parser.hpp"
#include <cctype>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <memory>
#include <vector>

namespace monty {
namespace syn {
std::unique_ptr<ast::ExprAST> Parser::parseExpression() noexcept {
  auto Lhs = parseUnary();
  if (!Lhs)
    return nullptr;

  return parseBinOpRhs(0, std::move(Lhs));
}

std::unique_ptr<ast::ExprAST> Parser::parseIdentifierExpr() noexcept {
  std::string idName = identifierStr;

  // eat identifier
  getNextToken();

  if (curToken != '(')
    return std::make_unique<ast::VariableExprAST>(idName);

  // Function call
  getNextToken();
  std::vector<std::unique_ptr<ast::ExprAST>> args;

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
  return std::make_unique<ast::FunctionCallExprAST>(idName, std::move(args));
}

std::unique_ptr<ast::ExprAST>
Parser::parseBinOpRhs(int exprPrec,
                      std::unique_ptr<ast::ExprAST> Lhs) noexcept {
  // find precedence of bin op
  while (true) {
    int tokenPrec = getTokenPrecedence();

    if (tokenPrec < exprPrec)
      return Lhs;

    // is a binary operation
    int binOp = curToken;
    getNextToken(); // eat bin op

    auto Rhs = parseUnary();
    if (!Rhs)
      return nullptr;

    int nextPrec = getTokenPrecedence();
    if (tokenPrec < nextPrec) {
      Rhs = parseBinOpRhs(tokenPrec + 1, std::move(Rhs));
      if (!Rhs)
        return nullptr;
    }

    // Combine Lhs and Rhs.
    Lhs = std::make_unique<ast::BinaryExprAST>(binOp, std::move(Lhs),
                                               std::move(Rhs));
  }
}

std::unique_ptr<ast::ExprAST> Parser::parseUnary() noexcept {
  // If the current token is not an operator, it must be a primary expr.
  if (!isascii(this->curToken) || this->curToken == '(' ||
      this->curToken == ',')
    return parsePrimery();

  // If this is a unary operator, read it.
  int opc = this->curToken;
  getNextToken();
  if (auto Operand = parseUnary())
    return std::make_unique<ast::UnaryExprAST>(opc, std::move(Operand));
  return nullptr;
}

std::unique_ptr<ast::ExprAST> Parser::parseIfExpr() noexcept {
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

  return std::make_unique<ast::IfExprAST>(std::move(cond), std::move(then),
                                          std::move(otherwise));
}

std::unique_ptr<ast::ExprAST> Parser::parseLetExpr() noexcept {
  getNextToken(); // eat the let

  std::vector<std::pair<std::string, std::unique_ptr<ast::ExprAST>>> varNames;

  // At least one variable name is required.
  if (this->curToken != token_identifier)
    return logError("expected identifier after let");

  while (true) {
    std::string name = this->identifierStr;
    getNextToken(); // eat identifier.

    // Read the optional initializer.
    std::unique_ptr<ast::ExprAST> init;
    if (this->curToken == '=') {
      getNextToken(); // eat the '='.

      init = parseExpression();
      if (!init)
        return nullptr;
    }

    varNames.push_back(std::make_pair(name, std::move(init)));

    // End of let list, exit loop.
    if (this->curToken != ',')
      break;
    getNextToken(); // eat the ','.

    if (this->curToken != token_identifier)
      return logError("expected identifier list after let");
  }

  // At this point, we have to have 'in'.
  if (this->curToken != token_in)
    return logError("expected 'in' keyword after 'let'");
  getNextToken(); // eat 'in'.

  auto body = parseExpression();
  if (!body)
    return nullptr;

  return std::make_unique<ast::LetExprAST>(std::move(varNames),
                                           std::move(body));
}

std::unique_ptr<ast::ExprAST> Parser::parsePrimery() noexcept {
  switch (curToken) {
  case token_identifier:
    return parseIdentifierExpr();
  case token_number:
    return parseNumberExpr();
  case '(':
    return parseParenExpr();
  case token_if:
    return parseIfExpr();
  case token_let:
    return parseLetExpr();
  case token_eof:
    return nullptr;
  default:
    return logError("Unknown token when expecting an expression");
  }
}

std::unique_ptr<ast::ExprAST> Parser::parseNumberExpr() noexcept {
  auto result = std::make_unique<ast::NumberExprAST>(numVal);
  getNextToken(); // consume the number
  return result;
}

std::unique_ptr<ast::FunctionPrototypeAST> Parser::parsePrototype() noexcept {
  std::string fnName;

  unsigned kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
  unsigned binaryPrecedence = 30;

  switch (this->curToken) {
  default:
    return logErrorP("Expected function name in prototype");
  case token_identifier:
    fnName = this->identifierStr;
    kind = 0;
    getNextToken();
    break;
  case token_unary:
    getNextToken();
    if (!isascii(this->curToken))
      return logErrorP("Expected unary operator");
    fnName = "unary";
    fnName += (char)this->curToken;
    kind = 1;
    getNextToken();
    break;
  case token_binary:
    getNextToken();
    if (!isascii(this->curToken))
      return logErrorP("Expected binary operator");
    fnName = "binary";
    fnName += (char)this->curToken;
    kind = 2;
    getNextToken();

    // Read the precedence if present.
    if (this->curToken == token_number) {
      if (this->numVal < 1 || this->numVal > 100)
        return logErrorP("Invalid precedence: must be 1..100");
      binaryPrecedence = (unsigned)this->numVal;
      getNextToken();
    }
    break;
  }

  if (this->curToken != '(')
    return logErrorP("Expected '(' in prototype");

  std::vector<std::string> argNames;
  while (getNextToken() == token_identifier)
    argNames.push_back(this->identifierStr);
  if (this->curToken != ')')
    return logErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'.

  // Verify right number of names for operator.
  if (kind && argNames.size() != kind)
    return logErrorP("Invalid number of operands for operator");

  return std::make_unique<ast::FunctionPrototypeAST>(
      fnName, std::move(argNames), kind != 0, binaryPrecedence);
}

std::unique_ptr<ast::FunctionAST> Parser::parseDefinition() noexcept {
  getNextToken(); // eat def.
  auto Proto = parsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = parseExpression())
    return std::make_unique<ast::FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

std::unique_ptr<ast::ExprAST> Parser::logError(const char *str) const noexcept {
  diag.report(str, curLoc);
  return nullptr;
}
std::unique_ptr<ast::FunctionPrototypeAST>
Parser::logErrorP(const char *Str) const noexcept {
  logError(Str);
  return nullptr;
}

std::unique_ptr<ast::FunctionPrototypeAST> Parser::parseExtern() noexcept {
  getNextToken(); // eat extern.
  return parsePrototype();
}

std::unique_ptr<ast::FunctionAST> Parser::parseTopLevelExpr() noexcept {
  if (auto expr = parseExpression()) {
    // Make an anonymous proto.
    auto proto = std::make_unique<ast::FunctionPrototypeAST>(
        "__anon_expr", std::vector<std::string>());
    return std::make_unique<ast::FunctionAST>(std::move(proto),
                                              std::move(expr));
  }
  return nullptr;
}

std::unique_ptr<ast::ExprAST> Parser::parseParenExpr() noexcept {
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
  // Skip any whitespace.
  while (isspace(lastChar))
    lastChar = getNextChar();

  // Store location of new token
  curLoc = {currentLine, currentCol};

  if (isalpha(lastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    identifierStr = lastChar;
    while (isalnum((lastChar = getNextChar())))
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
    if (identifierStr == "let")
      return token_let;
    if (identifierStr == "in")
      return token_in;

    return token_identifier;
  }

  if (isdigit(lastChar) || lastChar == '.') { // Number: [0-9.]+
    std::string numStr;
    do {
      numStr += lastChar;
      lastChar = getNextChar();
    } while (isdigit(lastChar) || lastChar == '.');

    numVal = strtod(numStr.c_str(), nullptr);
    return token_number;
  }

  if (lastChar == '#') {
    // Comment until end of line.
    do
      lastChar = getNextChar();
    while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

    if (lastChar != EOF)
      return getToken();
  }

  // Check for end of file.  Don't eat the EOF.
  if (lastChar == EOF)
    return token_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = lastChar;
  lastChar = getNextChar();
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

int Parser::getNextChar() noexcept {
  int c = this->inputStream.get();
  if (c == '\n') {
    currentLine++;
    currentCol = 0;
  } else {
    currentCol++;
  }

  return c;
}

void Parser::synchronize() noexcept {
  getNextToken(); // Advance to avoid getting stuck on the error token

  while (curToken != token_eof) {
    if (curToken == ';') {
      getNextToken();
      return;
    }

    // Check if at the start of a new declaration
    switch (curToken) {
    case token_def:
    case token_extern:
      return;
    }

    getNextToken();
  }
}

} // namespace syn
} // namespace monty
