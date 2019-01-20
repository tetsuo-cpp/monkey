#include "Parser.h"

#include <Token/Token.h>

namespace monkey {

Parser::Parser(Lexer &L) : L(L) {
  nextToken();
  nextToken();
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
  default:
    return nullptr;
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

  return false;
}

} // namespace monkey
