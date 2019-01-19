#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>

#include <memory>

namespace monkey {

class Parser {
public:
  Parser(Lexer &L);
  virtual ~Parser() = default;

  std::unique_ptr<Program> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<LetStatement> parseLetStatement();
  void nextToken();
  bool curTokenIs(TokenType) const;
  bool peekTokenIs(TokenType) const;
  bool expectPeek(TokenType);

private:
  Lexer L;
  Token CurToken;
  Token PeekToken;
};

} // namespace monkey
