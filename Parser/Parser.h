#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>

#include <memory>
#include <string>

namespace monkey {

class Parser {
public:
  Parser(Lexer &L);
  virtual ~Parser() = default;

  std::unique_ptr<Program> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<LetStatement> parseLetStatement();
  std::unique_ptr<ReturnStatement> parseReturnStatement();
  void nextToken();
  bool curTokenIs(TokenType) const;
  bool peekTokenIs(TokenType) const;
  bool expectPeek(TokenType);
  void peekError(TokenType);
  const std::vector<std::string> &errors() const;

private:
  Lexer L;
  Token CurToken;
  Token PeekToken;
  std::vector<std::string> Errors;
};

} // namespace monkey
