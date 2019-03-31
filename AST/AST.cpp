#include "AST.h"

#include <sstream>

namespace {

const std::string Empty;

} // namespace

namespace monkey::ast {

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

Identifier::Identifier(Token Tok, const std::string &Value)
    : Tok(Tok), Value(Value) {}

const std::string &Identifier::tokenLiteral() const { return Tok.Literal; }

std::string Identifier::string() const { return Value; }

LetStatement::LetStatement(Token Tok, std::unique_ptr<Identifier> Name,
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

ReturnStatement::ReturnStatement(Token Tok,
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

ExpressionStatement::ExpressionStatement(Token Tok,
                                         std::unique_ptr<Expression> Expr)
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

IntegerLiteral::IntegerLiteral(Token Tok, int64_t Value)
    : Tok(Tok), Value(Value) {}

const std::string &IntegerLiteral::tokenLiteral() const { return Tok.Literal; }

std::string IntegerLiteral::string() const { return Tok.Literal; }

Boolean::Boolean(Token Tok, bool Value) : Tok(Tok), Value(Value) {}

const std::string &Boolean::tokenLiteral() const { return Tok.Literal; }

std::string Boolean::string() const { return Tok.Literal; }

const std::string &String::tokenLiteral() const { return Tok.Literal; }

std::string String::string() const { return Tok.Literal; }

FunctionLiteral::FunctionLiteral(
    Token Tok, std::vector<std::unique_ptr<Identifier>> &&Parameters,
    std::unique_ptr<BlockStatement> Body)
    : Tok(Tok), Parameters(std::move(Parameters)), Body(std::move(Body)) {}

const std::string &FunctionLiteral::tokenLiteral() const { return Tok.Literal; }

std::string FunctionLiteral::string() const {
  std::stringstream SS;
  SS << tokenLiteral();
  SS << "(";
  for (const auto &Param : Parameters) {
    SS << Param->string();
    if (Param.get() != Parameters.back().get()) {
      SS << ", ";
    }
  }

  SS << ")";
  SS << Body->string();
  return SS.str();
}

PrefixExpression::PrefixExpression(Token Tok, const std::string &Operator,
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

BlockStatement::BlockStatement(
    Token Tok, std::vector<std::unique_ptr<Statement>> &&Statements)
    : Tok(Tok), Statements(std::move(Statements)) {}

const std::string &BlockStatement::tokenLiteral() const { return Tok.Literal; }

std::string BlockStatement::string() const {
  std::stringstream SS;
  for (const auto &S : Statements) {
    SS << S->string();
  }

  return SS.str();
}

IfExpression::IfExpression(Token Tok, std::unique_ptr<Expression> Condition,
                           std::unique_ptr<BlockStatement> Consequence,
                           std::unique_ptr<BlockStatement> Alternative)
    : Tok(Tok), Condition(std::move(Condition)),
      Consequence(std::move(Consequence)), Alternative(std::move(Alternative)) {
}

const std::string &IfExpression::tokenLiteral() const { return Tok.Literal; }

std::string IfExpression::string() const {
  std::stringstream SS;
  SS << "if";
  SS << Condition->string();
  SS << " ";
  SS << Consequence->string();
  if (Alternative) {
    SS << "else ";
    SS << Alternative->string();
  }

  return SS.str();
}

CallExpression::CallExpression(
    Token Tok, std::unique_ptr<Expression> Function,
    std::vector<std::unique_ptr<Expression>> &&Arguments)
    : Tok(Tok), Function(std::move(Function)), Arguments(std::move(Arguments)) {
}

const std::string &CallExpression::tokenLiteral() const { return Tok.Literal; }

std::string CallExpression::string() const {
  std::stringstream SS;
  SS << Function->string();
  SS << "(";
  for (const auto &Arg : Arguments) {
    SS << Arg->string();
    if (Arg.get() != Arguments.back().get()) {
      SS << ", ";
    }
  }

  SS << ")";
  return SS.str();
}

ArrayLiteral::ArrayLiteral(Token Tok,
                           std::vector<std::unique_ptr<Expression>> &&Elements)
    : Tok(Tok), Elements(std::move(Elements)) {}

const std::string &ArrayLiteral::tokenLiteral() const { return Tok.Literal; }

std::string ArrayLiteral::string() const {
  std::stringstream SS;
  SS << "[";
  for (const auto &E : Elements) {
    SS << E->string();
    if (E.get() != Elements.back().get()) {
      SS << ", ";
    }
  }

  SS << "]";
  return SS.str();
}

IndexExpression::IndexExpression(Token Tok, std::unique_ptr<Expression> Left,
                                 std::unique_ptr<Expression> Index)
    : Tok(Tok), Left(std::move(Left)), Index(std::move(Index)) {}

const std::string &IndexExpression::tokenLiteral() const { return Tok.Literal; }

std::string IndexExpression::string() const {
  return "(" + Left->string() + "[" + Index->string() + "])";
}

} // namespace monkey::ast
