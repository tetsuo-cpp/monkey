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

Identifier::Identifier(struct Token Token, const std::string &Value)
    : Token(Token), Value(Value) {}

const std::string &Identifier::tokenLiteral() const { return Token.Literal; }

std::string Identifier::string() const { return Value; }

LetStatement::LetStatement(struct Token Token,
                           std::unique_ptr<Identifier> &&Name,
                           std::unique_ptr<Expression> &&Value)
    : Token(Token), Name(std::move(Name)), Value(std::move(Value)) {}

const std::string &LetStatement::tokenLiteral() const { return Token.Literal; }

std::string LetStatement::string() const {
  std::stringstream SS;
  SS << tokenLiteral() << " " << Name->string() << " = ";
  if (Value) {
    SS << Value->string();
  }

  SS << ";";
  return SS.str();
}

ReturnStatement::ReturnStatement(struct Token Token,
                                 std::unique_ptr<Expression> &&ReturnValue)
    : Token(Token), ReturnValue(std::move(ReturnValue)) {}

const std::string &ReturnStatement::tokenLiteral() const {
  return Token.Literal;
}

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
    struct Token Token, std::unique_ptr<struct Expression> &&Expression)
    : Token(Token), Expression(std::move(Expression)) {}

const std::string &ExpressionStatement::tokenLiteral() const {
  return Token.Literal;
}

std::string ExpressionStatement::string() const {
  if (Expression) {
    return Expression->string();
  }

  return std::string();
}

} // namespace monkey
