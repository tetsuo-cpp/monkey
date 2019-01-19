#pragma once

#include <Token/Token.h>

#include <memory>
#include <string>
#include <vector>

namespace monkey {

struct Node {
  virtual ~Node() = default;

  virtual const std::string &tokenLiteral() const = 0;
};

struct Statement : public Node {
  virtual ~Statement() = default;
};

struct Expression : public Node {
  virtual ~Expression() = default;
};

struct Program : public Node {
  virtual ~Program() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;

  std::vector<std::unique_ptr<Statement>> Statements;
};

struct Identifier : public Expression {
  virtual ~Identifier() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;

  Token Token;
  std::string Value;
};

struct LetStatement : public Statement {
  virtual ~LetStatement() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;

  Token Token;
  std::unique_ptr<Identifier> Name;
  std::unique_ptr<Expression> Value;
};

} // namespace monkey
