#include "Evaluator.h"

namespace monkey::evaluator {

namespace {

std::unique_ptr<object::Object>
evalStatements(const std::vector<std::unique_ptr<ast::Statement>> &Statements) {
  std::unique_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get());
  }

  return Result;
}

std::unique_ptr<object::Object>
evalBangOperatorExpression(const std::unique_ptr<object::Object> &Right) {
  auto *Boolean = dynamic_cast<object::Boolean *>(Right.get());
  if (Boolean) {
    return std::make_unique<object::Boolean>(!Boolean->Value);
  }

  auto *Null = dynamic_cast<object::Null *>(Right.get());
  if (Null) {
    return std::make_unique<object::Boolean>(true);
  }

  return std::make_unique<object::Boolean>(false);
}

std::unique_ptr<object::Object> evalMinusPrefixOperatorExpression(
    const std::unique_ptr<object::Object> &Right) {
  auto *Integer = dynamic_cast<object::Integer *>(Right.get());
  if (!Integer) {
    return nullptr;
  }

  return std::make_unique<object::Integer>(-Integer->Value);
}

std::unique_ptr<object::Object>
evalPrefixExpression(const std::string &Operator,
                     const std::unique_ptr<object::Object> &Right) {
  if (Operator == "!") {
    return evalBangOperatorExpression(Right);
  } else if (Operator == "-") {
    return evalMinusPrefixOperatorExpression(Right);
  } else {
    return nullptr;
  }
}

std::unique_ptr<object::Object>
evalIntegerInfixExpression(const std::string &Operator,
                           const std::unique_ptr<object::Object> &Left,
                           const std::unique_ptr<object::Object> &Right) {
  const auto *LeftInt = dynamic_cast<object::Integer *>(Left.get());
  assert(LeftInt);
  const auto *RightInt = dynamic_cast<object::Integer *>(Right.get());
  assert(RightInt);

  if (Operator == "+") {
    return std::make_unique<object::Integer>(LeftInt->Value + RightInt->Value);
  } else if (Operator == "-") {
    return std::make_unique<object::Integer>(LeftInt->Value - RightInt->Value);
  } else if (Operator == "*") {
    return std::make_unique<object::Integer>(LeftInt->Value * RightInt->Value);
  } else if (Operator == "/") {
    return std::make_unique<object::Integer>(LeftInt->Value / RightInt->Value);
  } else if (Operator == "<") {
    return std::make_unique<object::Boolean>(LeftInt->Value < RightInt->Value);
  } else if (Operator == ">") {
    return std::make_unique<object::Boolean>(LeftInt->Value > RightInt->Value);
  } else if (Operator == "==") {
    return std::make_unique<object::Boolean>(LeftInt->Value == RightInt->Value);
  } else if (Operator == "!=") {
    return std::make_unique<object::Boolean>(LeftInt->Value != RightInt->Value);
  } else {
    return nullptr;
  }
}

std::unique_ptr<object::Object>
evalInfixExpression(const std::string &Operator,
                    const std::unique_ptr<object::Object> &Left,
                    const std::unique_ptr<object::Object> &Right) {
  if (Left->type() == object::INTEGER_OBJ &&
      Right->type() == object::INTEGER_OBJ) {
    return evalIntegerInfixExpression(Operator, Left, Right);
  } else {
    return nullptr;
  }
}

} // namespace

// TODO: Maybe switch to using a visitor to avoid the constant dynamic
// casting?
std::unique_ptr<object::Object> eval(const ast::Node *Node) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program) {
    return evalStatements(Program->Statements);
  }

  const auto *ExprS = dynamic_cast<const ast::ExpressionStatement *>(Node);
  if (ExprS) {
    return eval(ExprS->Expr.get());
  }

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL) {
    return std::make_unique<object::Integer>(IntegerL->Value);
  }

  const auto *BooleanL = dynamic_cast<const ast::Boolean *>(Node);
  if (BooleanL) {
    return std::make_unique<object::Boolean>(BooleanL->Value);
  }

  const auto *PrefixE = dynamic_cast<const ast::PrefixExpression *>(Node);
  if (PrefixE) {
    auto Right = eval(PrefixE->Right.get());
    return evalPrefixExpression(PrefixE->Operator, Right);
  }

  const auto *InfixE = dynamic_cast<const ast::InfixExpression *>(Node);
  if (InfixE) {
    auto Left = eval(InfixE->Left.get());
    auto Right = eval(InfixE->Right.get());
    return evalInfixExpression(InfixE->Operator, Left, Right);
  }

  return nullptr;
}

} // namespace monkey::evaluator
