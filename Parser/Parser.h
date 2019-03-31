#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace monkey::parser {

using PrefixParseFn = std::function<std::unique_ptr<ast::Expression>()>;
using InfixParseFn = std::function<std::unique_ptr<ast::Expression>(
    std::unique_ptr<ast::Expression>)>;

enum class Precedence {
  LOWEST,
  EQUALS,
  LESSGREATER,
  SUM,
  PRODUCT,
  PREFIX,
  CALL,
  INDEX
};

class Parser {
public:
  Parser(lexer::Lexer &L);
  virtual ~Parser() = default;

  std::unique_ptr<ast::Program> parseProgram();
  const std::vector<std::string> &errors() const;

private:
  std::unique_ptr<ast::Statement> parseStatement();
  std::unique_ptr<ast::LetStatement> parseLetStatement();
  std::unique_ptr<ast::ReturnStatement> parseReturnStatement();
  std::unique_ptr<ast::ExpressionStatement> parseExpressionStatement();
  std::unique_ptr<ast::Expression> parseExpression(Precedence);
  std::unique_ptr<ast::Expression> parseIdentifier();
  std::unique_ptr<ast::IntegerLiteral> parseIntegerLiteral();
  std::unique_ptr<ast::Expression> parsePrefixExpression();
  std::unique_ptr<ast::Expression>
      parseInfixExpression(std::unique_ptr<ast::Expression>);
  std::unique_ptr<ast::Expression> parseBoolean();
  std::unique_ptr<ast::Expression> parseStringLiteral();
  std::unique_ptr<ast::Expression> parseGroupedExpression();
  std::unique_ptr<ast::Expression> parseIfExpression();
  std::unique_ptr<ast::BlockStatement> parseBlockStatement();
  std::unique_ptr<ast::Expression> parseFunctionLiteral();
  std::vector<std::unique_ptr<ast::Identifier>> parseFunctionParameters();
  std::unique_ptr<ast::Expression>
      parseCallExpression(std::unique_ptr<ast::Expression>);
  std::unique_ptr<ast::Expression> parseArrayLiteral();
  std::vector<std::unique_ptr<ast::Expression>> parseExpressionList(TokenType);
  std::unique_ptr<ast::Expression>
      parseIndexExpression(std::unique_ptr<ast::Expression>);
  void nextToken();
  bool curTokenIs(TokenType) const;
  bool peekTokenIs(TokenType) const;
  bool expectPeek(TokenType);
  void peekError(TokenType);
  void registerPrefix(TokenType, PrefixParseFn);
  void registerInfix(TokenType, InfixParseFn);
  void noPrefixParseFnError(TokenType);
  Precedence peekPrecedence() const;
  Precedence curPrecedence() const;

  lexer::Lexer L;
  Token CurToken;
  Token PeekToken;
  std::vector<std::string> Errors;
  std::map<TokenType, PrefixParseFn> PrefixParseFns;
  std::map<TokenType, InfixParseFn> InfixParseFns;
};

} // namespace monkey::parser
