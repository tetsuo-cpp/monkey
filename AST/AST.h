#pragma once

#include <Token/Token.h>

#include <memory>
#include <string>
#include <vector>

namespace monkey {

struct Node {
  virtual ~Node() = default;

  virtual const std::string &tokenLiteral() const = 0;
  virtual std::string string() const = 0;
};

struct Statement : public Node {
  virtual ~Statement() = default;
};

struct Expression : public Node {
  virtual ~Expression() = default;
};

struct Program : public Node {
  Program() = default;
  explicit Program(std::vector<std::unique_ptr<Statement>> &&);
  virtual ~Program() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  std::vector<std::unique_ptr<Statement>> Statements;
};

struct Identifier : public Expression {
  Identifier() = default;
  Identifier(Token, const std::string &);
  virtual ~Identifier() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::string Value;
};

struct LetStatement : public Statement {
  LetStatement() = default;
  LetStatement(Token, std::unique_ptr<Identifier>, std::unique_ptr<Expression>);
  virtual ~LetStatement() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unique_ptr<Identifier> Name;
  std::unique_ptr<Expression> Value;
};

struct ReturnStatement : public Statement {
  ReturnStatement() = default;
  ReturnStatement(Token, std::unique_ptr<Expression>);
  virtual ~ReturnStatement() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unique_ptr<Expression> ReturnValue;
};

struct ExpressionStatement : public Statement {
  ExpressionStatement() = default;
  ExpressionStatement(Token, std::unique_ptr<Expression>);
  virtual ~ExpressionStatement() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unique_ptr<Expression> Expr;
};

struct IntegerLiteral : public Expression {
  IntegerLiteral() = default;
  IntegerLiteral(Token, int64_t);
  virtual ~IntegerLiteral() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  int64_t Value;
};

struct PrefixExpression : public Expression {
  PrefixExpression() = default;
  PrefixExpression(Token, const std::string &, std::unique_ptr<Expression>);
  virtual ~PrefixExpression() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::string Operator;
  std::unique_ptr<Expression> Right;
};

struct InfixExpression : public Expression {
  InfixExpression() = default;
  InfixExpression(Token, const std::string &, std::unique_ptr<Expression>,
                  std::unique_ptr<Expression>);
  virtual ~InfixExpression() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::string Operator;
  std::unique_ptr<Expression> Left, Right;
};

} // namespace monkey
