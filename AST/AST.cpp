#include "AST.h"

#include <sstream>

namespace {

const std::string Empty;

} // namespace

namespace monkey {

Program::Program(std::vector<std::unique_ptr<Statement>> &&Statements)
    : Statements(std::move(Statements)) {}

const std::string &Program::tokenLiteral() const {
  if (!Statements.empty()) {
    return Statements.front()->tokenLiteral();
  } else {
    return Empty;
  }
}

std::string Program::string() const {
  std::stringstream SS;
  for (const auto &Statement : Statements) {
    SS << Statement->string();
  }

  return SS.str();
}

Identifier::Identifier(struct Token Tok, const std::string &Value)
    : Tok(Tok), Value(Value) {}

const std::string &Identifier::tokenLiteral() const { return Tok.Literal; }

std::string Identifier::string() const { return Value; }

LetStatement::LetStatement(struct Token Tok, std::unique_ptr<Identifier> Name,
                           std::unique_ptr<Expression> Value)
    : Tok(Tok), Name(std::move(Name)), Value(std::move(Value)) {}

const std::string &LetStatement::tokenLiteral() const { return Tok.Literal; }

std::string LetStatement::string() const {
  std::stringstream SS;
  SS << tokenLiteral() << " " << Name->string() << " = ";
  if (Value) {
    SS << Value->string();
  }

  SS << ";";
  return SS.str();
}

ReturnStatement::ReturnStatement(struct Token Tok,
                                 std::unique_ptr<Expression> ReturnValue)
    : Tok(Tok), ReturnValue(std::move(ReturnValue)) {}

const std::string &ReturnStatement::tokenLiteral() const { return Tok.Literal; }

std::string ReturnStatement::string() const {
  std::stringstream SS;
  SS << tokenLiteral() << " ";
  if (ReturnValue) {
    SS << ReturnValue->string();
  }

  SS << ";";
  return SS.str();
}

ExpressionStatement::ExpressionStatement(
    struct Token Tok, std::unique_ptr<struct Expression> Expr)
    : Tok(Tok), Expr(std::move(Expr)) {}

const std::string &ExpressionStatement::tokenLiteral() const {
  return Tok.Literal;
}

std::string ExpressionStatement::string() const {
  if (Expr) {
    return Expr->string();
  }

  return std::string();
}

IntegerLiteral::IntegerLiteral(struct Token Tok, int64_t Value)
    : Tok(Tok), Value(Value) {}

const std::string &IntegerLiteral::tokenLiteral() const { return Tok.Literal; }

std::string IntegerLiteral::string() const { return Tok.Literal; }

PrefixExpression::PrefixExpression(struct Token Tok,
                                   const std::string &Operator,
                                   std::unique_ptr<Expression> Right)
    : Tok(Tok), Operator(Operator), Right(std::move(Right)) {}

const std::string &PrefixExpression::tokenLiteral() const {
  return Tok.Literal;
}

std::string PrefixExpression::string() const {
  std::stringstream SS;
  SS << "(";
  SS << Operator;
  SS << Right->string();
  SS << ")";
  return SS.str();
}

InfixExpression::InfixExpression(Token Tok, const std::string &Operator,
                                 std::unique_ptr<Expression> Left,
                                 std::unique_ptr<Expression> Right)
    : Tok(Tok), Operator(Operator), Left(std::move(Left)),
      Right(std::move(Right)) {}

const std::string &InfixExpression::tokenLiteral() const { return Tok.Literal; }

std::string InfixExpression::string() const {
  std::stringstream SS;
  SS << "(";
  SS << Left->string();
  SS << " " << Operator << " ";
  SS << Right->string();
  SS << ")";
  return SS.str();
}

} // namespace monkey
