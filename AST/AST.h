#pragma once

#include <Token/Token.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace monkey::ast {

enum class ASTType : uint64_t {
  PROGRAM_AST,
  IDENTIFIER_AST,
  LET_AST,
  RETURN_AST,
  EXPR_STATEMENT_AST,
  INTEGER_AST,
  BOOLEAN_AST,
  STRING_AST,
  FUNCTION_AST,
  PREFIX_EXPR_AST,
  INFIX_EXPR_AST,
  BLOCK_AST,
  IF_AST,
  CALL_AST,
  ARRAY_AST,
  INDEX_AST,
  HASH_AST
};

struct BlockStatement;

struct Node {
  virtual ~Node() = default;

  virtual const std::string &tokenLiteral() const = 0;
  virtual std::string string() const = 0;
  virtual ASTType type() const = 0;
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
  ASTType type() const override;

  std::vector<std::unique_ptr<Statement>> Statements;
};

struct Identifier : public Expression {
  Identifier() = default;
  Identifier(Token, const std::string &);
  virtual ~Identifier() = default;

  // Node impl.
  const std::string &tokenLiteral() const override;
  std::string string() const override;
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

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
  ASTType type() const override;

  Token Tok;
  std::vector<
      std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>
      Pairs;
};

template <typename T, ASTType Type> inline T astCastImpl(const Node *Node) {
  if (Node && Node->type() == Type)
    return static_cast<T>(Node);
  else
    return nullptr;
}

template <typename T> inline T astCast(const Node *) {
  static_assert(sizeof(T) != sizeof(T),
                "astCast must be specialised for this type");
}

template <> inline const Program *astCast<const Program *>(const Node *Node) {
  return astCastImpl<const Program *, ASTType::PROGRAM_AST>(Node);
}

template <>
inline const Identifier *astCast<const Identifier *>(const Node *Node) {
  return astCastImpl<const Identifier *, ASTType::IDENTIFIER_AST>(Node);
}

template <>
inline const LetStatement *astCast<const LetStatement *>(const Node *Node) {
  return astCastImpl<const LetStatement *, ASTType::LET_AST>(Node);
}

template <>
inline const ReturnStatement *
astCast<const ReturnStatement *>(const Node *Node) {
  return astCastImpl<const ReturnStatement *, ASTType::RETURN_AST>(Node);
}

template <>
inline const ExpressionStatement *
astCast<const ExpressionStatement *>(const Node *Node) {
  return astCastImpl<const ExpressionStatement *, ASTType::EXPR_STATEMENT_AST>(
      Node);
}

template <>
inline const IntegerLiteral *astCast<const IntegerLiteral *>(const Node *Node) {
  return astCastImpl<const IntegerLiteral *, ASTType::INTEGER_AST>(Node);
}

template <> inline const Boolean *astCast<const Boolean *>(const Node *Node) {
  return astCastImpl<const Boolean *, ASTType::BOOLEAN_AST>(Node);
}

template <> inline const String *astCast<const String *>(const Node *Node) {
  return astCastImpl<const String *, ASTType::STRING_AST>(Node);
}

template <>
inline FunctionLiteral *astCast<FunctionLiteral *>(const Node *Node) {
  return const_cast<FunctionLiteral *>(
      astCastImpl<const FunctionLiteral *, ASTType::FUNCTION_AST>(Node));
}

template <>
inline const PrefixExpression *
astCast<const PrefixExpression *>(const Node *Node) {
  return astCastImpl<const PrefixExpression *, ASTType::PREFIX_EXPR_AST>(Node);
}

template <>
inline const InfixExpression *
astCast<const InfixExpression *>(const Node *Node) {
  return astCastImpl<const InfixExpression *, ASTType::INFIX_EXPR_AST>(Node);
}

template <>
inline const BlockStatement *astCast<const BlockStatement *>(const Node *Node) {
  return astCastImpl<const BlockStatement *, ASTType::BLOCK_AST>(Node);
}

template <>
inline const IfExpression *astCast<const IfExpression *>(const Node *Node) {
  return astCastImpl<const IfExpression *, ASTType::IF_AST>(Node);
}

template <>
inline const CallExpression *astCast<const CallExpression *>(const Node *Node) {
  return astCastImpl<const CallExpression *, ASTType::CALL_AST>(Node);
}

template <>
inline const ArrayLiteral *astCast<const ArrayLiteral *>(const Node *Node) {
  return astCastImpl<const ArrayLiteral *, ASTType::ARRAY_AST>(Node);
}

template <>
inline const IndexExpression *
astCast<const IndexExpression *>(const Node *Node) {
  return astCastImpl<const IndexExpression *, ASTType::INDEX_AST>(Node);
}

template <>
inline const HashLiteral *astCast<const HashLiteral *>(const Node *Node) {
  return astCastImpl<const HashLiteral *, ASTType::HASH_AST>(Node);
}

} // namespace monkey::ast
