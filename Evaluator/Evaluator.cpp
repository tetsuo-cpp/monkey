#include "Evaluator.h"

#include <cassert>

namespace monkey::evaluator {

namespace {

std::unique_ptr<object::Error> newError(const char *Format, ...) {
#define ERROR_SIZE 1024
  char Error[ERROR_SIZE];
  va_list ArgList;
  va_start(ArgList, Format);
  vsnprintf(Error, ERROR_SIZE, Format, ArgList);
  va_end(ArgList);
  return std::make_unique<object::Error>(Error);
}

bool isError(const std::unique_ptr<object::Object> &Obj) {
  return Obj && Obj->type() == object::ERROR_OBJ;
}

std::unique_ptr<object::Object>
evalProgram(const std::vector<std::unique_ptr<ast::Statement>> &Statements) {
  std::unique_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get());
    auto *ReturnV = dynamic_cast<object::ReturnValue *>(Result.get());
    if (ReturnV) {
      return std::move(ReturnV->Value);
    }

    auto *ErrorV = dynamic_cast<object::Error *>(Result.get());
    if (ErrorV) {
      return Result;
    }
  }

  return Result;
}

std::unique_ptr<object::Object> evalBlockStatement(
    const std::vector<std::unique_ptr<ast::Statement>> &Statements) {
  std::unique_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get());
    if (Result) {
      const auto &Type = Result->type();
      if (Type == object::RETURN_VALUE_OBJ || Type == object::ERROR_OBJ) {
        return Result;
      }
    }
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
  if (Right->type() != object::INTEGER_OBJ) {
    return newError("unknown operator: -%s", Right->type().c_str());
  }

  auto *Integer = dynamic_cast<object::Integer *>(Right.get());
  if (!Integer) {
    return std::make_unique<object::Null>();
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
    return newError("unknown operator: %s:%s", Operator.c_str(),
                    Right->type().c_str());
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
    return newError("unknown operator: %s %s %s", Left->type().c_str(),
                    Operator.c_str(), Right->type().c_str());
  }
}

std::unique_ptr<object::Object>
evalBooleanInfixExpression(const std::string &Operator,
                           const std::unique_ptr<object::Object> &Left,
                           const std::unique_ptr<object::Object> &Right) {
  const bool BothEqual = [&Left, &Right]() {
    if (Left->type() == object::BOOLEAN_OBJ ^
        Right->type() == object::BOOLEAN_OBJ) {
      return false;
    }

    const auto *L = dynamic_cast<object::Boolean *>(Left.get());
    const auto *R = dynamic_cast<object::Boolean *>(Right.get());
    assert(L);
    assert(R);

    return L->Value == R->Value;
  }();

  if (Operator == "==") {
    return std::make_unique<object::Boolean>(BothEqual);
  } else if (Operator == "!=") {
    return std::make_unique<object::Boolean>(!BothEqual);
  } else {
    if (Left->type() == object::BOOLEAN_OBJ ^
        Right->type() == object::BOOLEAN_OBJ) {
      return newError("type mismatch: %s %s %s", Left->type().c_str(),
                      Operator.c_str(), Right->type().c_str());
    } else {
      return newError("unknown operator: %s %s %s", Left->type().c_str(),
                      Operator.c_str(), Right->type().c_str());
    }
  }
}

std::unique_ptr<object::Object>
evalNullInfixExpression(const std::string &Operator,
                        const std::unique_ptr<object::Object> &Left,
                        const std::unique_ptr<object::Object> &Right) {
  const bool BothNull =
      Left->type() == object::NULL_OBJ && Right->type() == object::NULL_OBJ;

  if (Operator == "==") {
    return std::make_unique<object::Boolean>(BothNull);
  } else if (Operator == "!=") {
    return std::make_unique<object::Boolean>(!BothNull);
  } else {
    return std::make_unique<object::Null>();
  }
}

std::unique_ptr<object::Object>
evalInfixExpression(const std::string &Operator,
                    const std::unique_ptr<object::Object> &Left,
                    const std::unique_ptr<object::Object> &Right) {
  if (Left->type() == object::INTEGER_OBJ &&
      Right->type() == object::INTEGER_OBJ) {
    return evalIntegerInfixExpression(Operator, Left, Right);
  } else if (Left->type() == object::NULL_OBJ ||
             Right->type() == object::NULL_OBJ) {
    return evalNullInfixExpression(Operator, Left, Right);
  } else if (Left->type() == object::BOOLEAN_OBJ ||
             Right->type() == object::BOOLEAN_OBJ) {
    return evalBooleanInfixExpression(Operator, Left, Right);
  } else if (Left->type() != Right->type()) {
    return newError("type mismatch: %s %s %s", Left->type().c_str(),
                    Operator.c_str(), Right->type().c_str());
  } else {
    return newError("unknown operator: %s %s %s", Left->type().c_str(),
                    Operator.c_str(), Right->type().c_str());
  }
}

bool isTruthy(const object::Object *Obj) {
  const auto *NullObj = dynamic_cast<const object::Null *>(Obj);
  if (NullObj) {
    return false;
  }

  const auto *BooleanObj = dynamic_cast<const object::Boolean *>(Obj);
  if (BooleanObj) {
    return BooleanObj->Value;
  }

  return true;
}

std::unique_ptr<object::Object>
evalIfExpression(const ast::IfExpression *Node) {
  auto Cond = eval(Node->Condition.get());
  if (isError(Cond)) {
    return Cond;
  }

  if (isTruthy(Cond.get())) {
    return eval(Node->Consequence.get());
  } else if (Node->Alternative) {
    return eval(Node->Alternative.get());
  } else {
    return std::make_unique<object::Null>();
  }
}

} // namespace

// TODO: Maybe switch to using a visitor to avoid the constant dynamic casting?
std::unique_ptr<object::Object> eval(const ast::Node *Node) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program) {
    return evalProgram(Program->Statements);
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
    if (isError(Right)) {
      return Right;
    }

    return evalPrefixExpression(PrefixE->Operator, Right);
  }

  const auto *InfixE = dynamic_cast<const ast::InfixExpression *>(Node);
  if (InfixE) {
    auto Left = eval(InfixE->Left.get());
    if (isError(Left)) {
      return Left;
    }

    auto Right = eval(InfixE->Right.get());
    if (isError(Right)) {
      return Right;
    }

    return evalInfixExpression(InfixE->Operator, Left, Right);
  }

  const auto *BlockS = dynamic_cast<const ast::BlockStatement *>(Node);
  if (BlockS) {
    return evalBlockStatement(BlockS->Statements);
  }

  const auto *IfE = dynamic_cast<const ast::IfExpression *>(Node);
  if (IfE) {
    return evalIfExpression(IfE);
  }

  const auto *ReturnS = dynamic_cast<const ast::ReturnStatement *>(Node);
  if (ReturnS) {
    auto Value = eval(ReturnS->ReturnValue.get());
    if (isError(Value)) {
      return Value;
    }

    return std::make_unique<object::ReturnValue>(std::move(Value));
  }

  return nullptr;
}

} // namespace monkey::evaluator
