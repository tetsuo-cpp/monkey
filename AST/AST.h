#pragma once

#include <Token/Token.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace monkey::ast {

struct BlockStatement;

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

struct Boolean : public Expression {
  Boolean() = default;
  Boolean(Token, bool);
  virtual ~Boolean() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  bool Value;
};

struct String : public Expression {
  template <typename T>
  explicit String(Token Tok, T &&Value)
      : Tok(Tok), Value(std::forward<T>(Value)) {}
  virtual ~String() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  const std::string Value;
};

struct FunctionLiteral : public Expression {
  FunctionLiteral() = default;
  FunctionLiteral(Token, std::vector<std::unique_ptr<Identifier>> &&,
                  std::unique_ptr<BlockStatement>);
  virtual ~FunctionLiteral() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::vector<std::unique_ptr<Identifier>> Parameters;
  std::unique_ptr<BlockStatement> Body;
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

struct BlockStatement : public Statement {
  BlockStatement() = default;
  BlockStatement(Token, std::vector<std::unique_ptr<Statement>> &&);
  virtual ~BlockStatement() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::vector<std::unique_ptr<Statement>> Statements;
};

struct IfExpression : public Expression {
  IfExpression() = default;
  IfExpression(Token, std::unique_ptr<Expression>,
               std::unique_ptr<BlockStatement>,
               std::unique_ptr<BlockStatement>);
  virtual ~IfExpression() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok; // The 'if' token.
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<BlockStatement> Consequence, Alternative;
};

struct CallExpression : public Expression {
  CallExpression() = default;
  CallExpression(Token, std::unique_ptr<Expression>,
                 std::vector<std::unique_ptr<Expression>> &&);
  virtual ~CallExpression() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unique_ptr<Expression> Function;
  std::vector<std::unique_ptr<Expression>> Arguments;
};

struct ArrayLiteral : public Expression {
  ArrayLiteral() = default;
  ArrayLiteral(Token, std::vector<std::unique_ptr<Expression>> &&);
  virtual ~ArrayLiteral() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::vector<std::unique_ptr<Expression>> Elements;
};

struct IndexExpression : public Expression {
  IndexExpression() = default;
  IndexExpression(Token, std::unique_ptr<Expression>,
                  std::unique_ptr<Expression>);
  ~IndexExpression() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unique_ptr<Expression> Left;
  std::unique_ptr<Expression> Index;
};

class HashLiteral : public Expression {
public:
  HashLiteral(Token);
  virtual ~HashLiteral() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;

  Token Tok;
  std::unordered_map<std::unique_ptr<Expression>, std::unique_ptr<Expression>>
      Pairs;
};

} // namespace monkey::ast
