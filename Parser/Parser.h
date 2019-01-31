#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace monkey {

using PrefixParseFn = std::function<std::unique_ptr<Expression>()>;
using InfixParseFn =
    std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression> &)>;

enum class Precedence {
  LOWEST,
  EQUALS,
  LESSGREATER,
  SUM,
  PRODUCT,
  PREFIX,
  CALL
};

class Parser {
public:
  Parser(Lexer &L);
  virtual ~Parser() = default;

  std::unique_ptr<Program> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<LetStatement> parseLetStatement();
  std::unique_ptr<ReturnStatement> parseReturnStatement();
  std::unique_ptr<ExpressionStatement> parseExpressionStatement();
  std::unique_ptr<Expression> parseExpression(Precedence);
  std::unique_ptr<Expression> parseIdentifier();
  void nextToken();
  bool curTokenIs(TokenType) const;
  bool peekTokenIs(TokenType) const;
  bool expectPeek(TokenType);
  void peekError(TokenType);
  const std::vector<std::string> &errors() const;
  void registerPrefix(TokenType, PrefixParseFn);
  void registerInfix(TokenType, InfixParseFn);

private:
  Lexer L;
  Token CurToken;
  Token PeekToken;
  std::vector<std::string> Errors;
  std::map<TokenType, PrefixParseFn> PrefixParseFns;
  std::map<TokenType, InfixParseFn> InfixParseFns;
};

} // namespace monkey
