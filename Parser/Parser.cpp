#include "Parser.h"

#include <Token/Token.h>

#include <sstream>

namespace monkey {

Parser::Parser(Lexer &L) : L(L) {
  nextToken();
  nextToken();

  registerPrefix(TokenType::IDENT, [this]() { return parseIdentifier(); });
}

std::unique_ptr<Program> Parser::parseProgram() {
  auto P = std::make_unique<Program>();

  while (CurToken.Type != TokenType::END_OF_FILE) {
    auto S = parseStatement();
    if (S) {
      P->Statements.push_back(std::move(S));
    }

    nextToken();
  }

  return P;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  switch (CurToken.Type) {
  case TokenType::LET:
    return parseLetStatement();
  case TokenType::RETURN:
    return parseReturnStatement();
  default:
    return parseExpressionStatement();
  }
}

std::unique_ptr<LetStatement> Parser::parseLetStatement() {
  auto LS = std::make_unique<LetStatement>();
  LS->Token = CurToken;

  if (!expectPeek(TokenType::IDENT)) {
    return nullptr;
  }

  auto Name = std::make_unique<Identifier>();
  Name->Token = CurToken;
  Name->Value = CurToken.Literal;

  LS->Name = std::move(Name);

  if (!expectPeek(TokenType::ASSIGN)) {
    return nullptr;
  }

  // TODO: We're skipping the expressions until we encounter a semicolon.
  while (!curTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  nextToken();

  return LS;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
  auto RS = std::make_unique<ReturnStatement>();
  RS->Token = CurToken;

  nextToken();

  // TODO: We're skipping the expressions until we encounter a semicolon.
  while (!curTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  nextToken();

  return RS;
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
  auto ES = std::make_unique<ExpressionStatement>();
  ES->Token = CurToken;
  ES->Expression = parseExpression(Precedence::LOWEST);

  if (peekTokenIs(TokenType::SEMICOLON)) {
    nextToken();
  }

  return ES;
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence) {
  auto FnIter = PrefixParseFns.find(CurToken.Type);
  if (FnIter == PrefixParseFns.end()) {
    return nullptr;
  }

  auto LeftExp = FnIter->second();
  return LeftExp;
}

std::unique_ptr<Expression> Parser::parseIdentifier() {
  return std::make_unique<Identifier>(CurToken, CurToken.Literal);
}

void Parser::nextToken() {
  CurToken = PeekToken;
  PeekToken = L.nextToken();
}

bool Parser::curTokenIs(TokenType Type) const { return PeekToken.Type == Type; }

bool Parser::peekTokenIs(TokenType Type) const {
  return PeekToken.Type == Type;
}

bool Parser::expectPeek(TokenType Type) {
  if (peekTokenIs(Type)) {
    nextToken();
    return true;
  }

  peekError(Type);
  return false;
}

void Parser::peekError(TokenType Type) {
  std::stringstream SS;
  SS << "expected next token to be " << tokenTypeToString(Type) << ", got "
     << tokenTypeToString(PeekToken.Type) << " instead";

  Errors.push_back(SS.str());
}

const std::vector<std::string> &Parser::errors() const { return Errors; }

void Parser::registerPrefix(TokenType Type, PrefixParseFn Prefix) {
  PrefixParseFns.emplace(Type, Prefix);
}

void Parser::registerInfix(TokenType Type, InfixParseFn Infix) {
  InfixParseFns.emplace(Type, Infix);
}

} // namespace monkey
