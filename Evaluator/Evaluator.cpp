#include "Evaluator.h"

#include <cassert>

namespace monkey::evaluator {

namespace {

std::shared_ptr<object::Error> newError(const char *Format, ...) {
#define ERROR_SIZE 1024
  char Error[ERROR_SIZE];
  va_list ArgList;
  va_start(ArgList, Format);
  vsnprintf(Error, ERROR_SIZE, Format, ArgList);
  va_end(ArgList);
  return std::make_shared<object::Error>(Error);
}

const std::vector<std::pair<std::string, std::shared_ptr<object::BuiltIn>>>
    BuiltIns = {
        {"len",
         std::make_shared<object::BuiltIn>(
             [](const std::vector<std::shared_ptr<object::Object>> &Args)
                 -> std::shared_ptr<object::Object> {
               if (Args.size() != 1) {
                 return newError("wrong number of arguments. got=%d, want=1",
                                 Args.size());
               }

               const auto *String =
                   dynamic_cast<const object::String *>(Args.front().get());
               if (String) {
                 return std::make_shared<object::Integer>(String->Value.size());
               }

               const auto *Array =
                   dynamic_cast<const object::Array *>(Args.front().get());
               if (Array) {
                 return std::make_shared<object::Integer>(
                     Array->Elements.size());
               }

               return newError("argument to \"len\" not supported, got %s",
                               Args.front()->type().c_str());
             })},
        {"first",
         std::make_shared<object::BuiltIn>(
             [](const std::vector<std::shared_ptr<object::Object>> &Args)
                 -> std::shared_ptr<object::Object> {
               if (Args.size() != 1) {
                 return newError("wrong number of arguments. got=%d, want=1",
                                 Args.size());
               }

               if (Args.front()->type() != object::ARRAY_OBJ) {
                 return newError("argument to \"first\" must be ARRAY, got %s",
                                 Args.front()->type().c_str());
               }

               const auto *Array =
                   dynamic_cast<const object::Array *>(Args.front().get());
               assert(Array);
               if (!Array->Elements.empty()) {
                 return Array->Elements.front();
               }

               return std::make_shared<object::Null>();
             })},
        {"last",
         std::make_shared<object::BuiltIn>(
             [](const std::vector<std::shared_ptr<object::Object>> &Args)
                 -> std::shared_ptr<object::Object> {
               if (Args.size() != 1) {
                 return newError("wrong number of arguments. got=%d, want=1",
                                 Args.size());
               }

               if (Args.front()->type() != object::ARRAY_OBJ) {
                 return newError("argument to \"last\" must be ARRAY, got %s",
                                 Args.front()->type().c_str());
               }

               const auto *Array =
                   dynamic_cast<const object::Array *>(Args.front().get());
               assert(Array);
               if (!Array->Elements.empty()) {
                 return Array->Elements.back();
               }

               return std::make_shared<object::Null>();
             })},
        {"rest",
         std::make_shared<object::BuiltIn>(
             [](const std::vector<std::shared_ptr<object::Object>> &Args)
                 -> std::shared_ptr<object::Object> {
               if (Args.size() != 1) {
                 return newError("wrong number of arguments. got=%d, want=1",
                                 Args.size());
               }

               if (Args.front()->type() != object::ARRAY_OBJ) {
                 return newError("argument to \"rest\" must be ARRAY, got %s",
                                 Args.front()->type().c_str());
               }

               const auto *Array =
                   dynamic_cast<const object::Array *>(Args.front().get());
               assert(Array);
               if (!Array->Elements.empty()) {
                 std::vector<std::shared_ptr<object::Object>> Rest;
                 std::copy(Array->Elements.begin() + 1, Array->Elements.end(),
                           std::back_inserter(Rest));
                 return std::make_shared<object::Array>(std::move(Rest));
               }

               return std::make_shared<object::Null>();
             })},
        {"push",
         std::make_shared<object::BuiltIn>(
             [](const std::vector<std::shared_ptr<object::Object>> &Args)
                 -> std::shared_ptr<object::Object> {
               if (Args.size() != 2) {
                 return newError("wrong number of arguments. got=%d, want=2",
                                 Args.size());
               }

               if (Args.front()->type() != object::ARRAY_OBJ) {
                 return newError("argument to \"push\" must be ARRAY, got %s",
                                 Args.front()->type().c_str());
               }

               const auto *Array =
                   dynamic_cast<const object::Array *>(Args.front().get());
               assert(Array);
               if (!Array->Elements.empty()) {
                 auto Pushed = Array->Elements;
                 Pushed.push_back(Args.at(1));
                 return std::make_shared<object::Array>(std::move(Pushed));
               }

               return std::make_shared<object::Null>();
             })}};

bool isError(const std::shared_ptr<object::Object> &Obj) {
  return Obj && Obj->type() == object::ERROR_OBJ;
}

std::shared_ptr<object::Object>
evalProgram(const std::vector<std::unique_ptr<ast::Statement>> &Statements,
            environment::Environment &Env) {
  std::shared_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get(), Env);
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

std::shared_ptr<object::Object> evalBlockStatement(
    const std::vector<std::unique_ptr<ast::Statement>> &Statements,
    environment::Environment &Env) {
  std::shared_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get(), Env);
    if (Result) {
      const auto &Type = Result->type();
      if (Type == object::RETURN_VALUE_OBJ || Type == object::ERROR_OBJ) {
        return Result;
      }
    }
  }

  return Result;
}

std::shared_ptr<object::Object>
evalBangOperatorExpression(const std::shared_ptr<object::Object> &Right) {
  auto *Boolean = dynamic_cast<object::Boolean *>(Right.get());
  if (Boolean) {
    return std::make_shared<object::Boolean>(!Boolean->Value);
  }

  auto *Null = dynamic_cast<object::Null *>(Right.get());
  if (Null) {
    return std::make_shared<object::Boolean>(true);
  }

  return std::make_shared<object::Boolean>(false);
}

std::shared_ptr<object::Object> evalMinusPrefixOperatorExpression(
    const std::shared_ptr<object::Object> &Right) {
  if (Right->type() != object::INTEGER_OBJ) {
    return newError("unknown operator: -%s", Right->type().c_str());
  }

  auto *Integer = dynamic_cast<object::Integer *>(Right.get());
  if (!Integer) {
    return std::make_shared<object::Null>();
  }

  return std::make_shared<object::Integer>(-Integer->Value);
}

std::shared_ptr<object::Object>
evalPrefixExpression(const std::string &Operator,
                     const std::shared_ptr<object::Object> &Right) {
  if (Operator == "!") {
    return evalBangOperatorExpression(Right);
  } else if (Operator == "-") {
    return evalMinusPrefixOperatorExpression(Right);
  } else {
    return newError("unknown operator: %s:%s", Operator.c_str(),
                    Right->type().c_str());
  }
}

std::shared_ptr<object::Object>
evalIntegerInfixExpression(const std::string &Operator,
                           const std::shared_ptr<object::Object> &Left,
                           const std::shared_ptr<object::Object> &Right) {
  const auto *LeftInt = dynamic_cast<object::Integer *>(Left.get());
  assert(LeftInt);
  const auto *RightInt = dynamic_cast<object::Integer *>(Right.get());
  assert(RightInt);

  if (Operator == "+") {
    return std::make_shared<object::Integer>(LeftInt->Value + RightInt->Value);
  } else if (Operator == "-") {
    return std::make_shared<object::Integer>(LeftInt->Value - RightInt->Value);
  } else if (Operator == "*") {
    return std::make_shared<object::Integer>(LeftInt->Value * RightInt->Value);
  } else if (Operator == "/") {
    return std::make_shared<object::Integer>(LeftInt->Value / RightInt->Value);
  } else if (Operator == "<") {
    return std::make_shared<object::Boolean>(LeftInt->Value < RightInt->Value);
  } else if (Operator == ">") {
    return std::make_shared<object::Boolean>(LeftInt->Value > RightInt->Value);
  } else if (Operator == "==") {
    return std::make_shared<object::Boolean>(LeftInt->Value == RightInt->Value);
  } else if (Operator == "!=") {
    return std::make_shared<object::Boolean>(LeftInt->Value != RightInt->Value);
  } else {
    return newError("unknown operator: %s %s %s", Left->type().c_str(),
                    Operator.c_str(), Right->type().c_str());
  }
}

std::shared_ptr<object::Object>
evalBooleanInfixExpression(const std::string &Operator,
                           const std::shared_ptr<object::Object> &Left,
                           const std::shared_ptr<object::Object> &Right) {
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
    return std::make_shared<object::Boolean>(BothEqual);
  } else if (Operator == "!=") {
    return std::make_shared<object::Boolean>(!BothEqual);
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

std::shared_ptr<object::Object>
evalNullInfixExpression(const std::string &Operator,
                        const std::shared_ptr<object::Object> &Left,
                        const std::shared_ptr<object::Object> &Right) {
  const bool BothNull =
      Left->type() == object::NULL_OBJ && Right->type() == object::NULL_OBJ;

  if (Operator == "==") {
    return std::make_shared<object::Boolean>(BothNull);
  } else if (Operator == "!=") {
    return std::make_shared<object::Boolean>(!BothNull);
  } else {
    return std::make_shared<object::Null>();
  }
}

std::shared_ptr<object::Object>
evalStringInfixExpression(const std::string &Operator,
                          const std::shared_ptr<object::Object> &Left,
                          const std::shared_ptr<object::Object> &Right) {
  if (Operator != "+") {
    return newError("unknown operator: %s %s %s", Left->type().c_str(),
                    Operator.c_str(), Right->type().c_str());
  }

  const auto *LeftS = dynamic_cast<const object::String *>(Left.get());
  const auto *RightS = dynamic_cast<const object::String *>(Right.get());
  assert(LeftS);
  assert(RightS);
  return std::make_shared<object::String>(LeftS->Value + RightS->Value);
}

std::shared_ptr<object::Object>
evalInfixExpression(const std::string &Operator,
                    const std::shared_ptr<object::Object> &Left,
                    const std::shared_ptr<object::Object> &Right) {
  if (Left->type() == object::INTEGER_OBJ &&
      Right->type() == object::INTEGER_OBJ) {
    return evalIntegerInfixExpression(Operator, Left, Right);
  } else if (Left->type() == object::NULL_OBJ ||
             Right->type() == object::NULL_OBJ) {
    return evalNullInfixExpression(Operator, Left, Right);
  } else if (Left->type() == object::BOOLEAN_OBJ ||
             Right->type() == object::BOOLEAN_OBJ) {
    return evalBooleanInfixExpression(Operator, Left, Right);
  } else if (Left->type() == object::STRING_OBJ ||
             Right->type() == object::STRING_OBJ) {
    return evalStringInfixExpression(Operator, Left, Right);
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

std::shared_ptr<object::Object>
evalIfExpression(const ast::IfExpression *Node, environment::Environment &Env) {
  auto Cond = eval(Node->Condition.get(), Env);
  if (isError(Cond)) {
    return Cond;
  }

  if (isTruthy(Cond.get())) {
    return eval(Node->Consequence.get(), Env);
  } else if (Node->Alternative) {
    return eval(Node->Alternative.get(), Env);
  } else {
    return std::make_shared<object::Null>();
  }
}

std::shared_ptr<object::Object>
evalIdentifier(const ast::Identifier *Identifier,
               environment::Environment &Env) {
  const auto &Value = Env.get(Identifier->Value);
  if (Value) {
    return Value;
  }

  const auto BIter = std::find_if(
      BuiltIns.begin(), BuiltIns.end(),
      [Identifier](
          const std::pair<std::string, std::shared_ptr<object::BuiltIn>> &Fn) {
        return Fn.first == Identifier->Value;
      });

  if (BIter != BuiltIns.end()) {
    return BIter->second;
  }

  return newError("identifier not found: %s", Identifier->Value.c_str());
}

std::vector<std::shared_ptr<object::Object>>
evalExpressions(const std::vector<std::unique_ptr<ast::Expression>> &Arguments,
                environment::Environment &Env) {
  std::vector<std::shared_ptr<object::Object>> Results;

  for (auto &Arg : Arguments) {
    auto Evaluated = eval(Arg.get(), Env);
    if (isError(Evaluated)) {
      return {Evaluated};
    }

    Results.push_back(std::move(Evaluated));
  }

  return Results;
}

std::shared_ptr<object::Object>
evalArrayIndexExpression(const std::shared_ptr<object::Object> &Array,
                         const std::shared_ptr<object::Object> &Index) {
  const auto *ArrayObj = dynamic_cast<const object::Array *>(Array.get());
  assert(ArrayObj);
  const auto *Idx = dynamic_cast<const object::Integer *>(Index.get());
  assert(Idx);

  if (Idx->Value < 0 ||
      Idx->Value >= static_cast<int64_t>(ArrayObj->Elements.size())) {
    return std::make_shared<object::Null>();
  }

  return ArrayObj->Elements.at(Idx->Value);
}

std::shared_ptr<object::Object>
evalIndexExpression(const std::shared_ptr<object::Object> &Left,
                    const std::shared_ptr<object::Object> &Index) {
  if (Left->type() == object::ARRAY_OBJ &&
      Index->type() == object::INTEGER_OBJ) {
    return evalArrayIndexExpression(Left, Index);
  }

  return newError("index operator not supported: %s", Left->type().c_str());
}

environment::Environment
extendFunctionEnv(const object::Function *Fn,
                  const std::vector<std::shared_ptr<object::Object>> &Args) {
  environment::Environment Env(Fn->Env);
  assert(Fn->Parameters.size() == Args.size());
  for (size_t Index = 0; Index < Args.size(); ++Index) {
    const auto &ParamName = Fn->Parameters.at(Index)->Value;
    const auto &Arg = Args.at(Index);
    Env.set(ParamName, Arg);
  }

  return Env;
}

std::shared_ptr<object::Object>
unwrapReturnValue(const std::shared_ptr<object::Object> &Obj) {
  const auto *ReturnValue =
      dynamic_cast<const object::ReturnValue *>(Obj.get());
  if (ReturnValue) {
    return ReturnValue->Value;
  }

  return Obj;
}

std::shared_ptr<object::Object>
applyFunction(const std::shared_ptr<object::Object> &Fn,
              const std::vector<std::shared_ptr<object::Object>> &Args) {
  const auto *Function = dynamic_cast<object::Function *>(Fn.get());
  if (Function) {
    auto ExtendedEnv = extendFunctionEnv(Function, Args);
    auto Evaluated = eval(Function->Body.get(), ExtendedEnv);
    return unwrapReturnValue(Evaluated);
  }

  const auto *BuiltIn = dynamic_cast<object::BuiltIn *>(Fn.get());
  if (BuiltIn) {
    return BuiltIn->Fn(Args);
  }

  return newError("not a function %s", Fn->type().c_str());
}

} // namespace

// TODO: Maybe switch to using a visitor to avoid the constant dynamic casting?
std::shared_ptr<object::Object> eval(ast::Node *Node,
                                     environment::Environment &Env) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program) {
    return evalProgram(Program->Statements, Env);
  }

  const auto *ExprS = dynamic_cast<const ast::ExpressionStatement *>(Node);
  if (ExprS) {
    return eval(ExprS->Expr.get(), Env);
  }

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL) {
    return std::make_shared<object::Integer>(IntegerL->Value);
  }

  const auto *BooleanL = dynamic_cast<const ast::Boolean *>(Node);
  if (BooleanL) {
    return std::make_shared<object::Boolean>(BooleanL->Value);
  }

  const auto *PrefixE = dynamic_cast<const ast::PrefixExpression *>(Node);
  if (PrefixE) {
    auto Right = eval(PrefixE->Right.get(), Env);
    if (isError(Right)) {
      return Right;
    }

    return evalPrefixExpression(PrefixE->Operator, Right);
  }

  const auto *InfixE = dynamic_cast<const ast::InfixExpression *>(Node);
  if (InfixE) {
    auto Left = eval(InfixE->Left.get(), Env);
    if (isError(Left)) {
      return Left;
    }

    auto Right = eval(InfixE->Right.get(), Env);
    if (isError(Right)) {
      return Right;
    }

    return evalInfixExpression(InfixE->Operator, Left, Right);
  }

  const auto *BlockS = dynamic_cast<const ast::BlockStatement *>(Node);
  if (BlockS) {
    return evalBlockStatement(BlockS->Statements, Env);
  }

  const auto *IfE = dynamic_cast<const ast::IfExpression *>(Node);
  if (IfE) {
    return evalIfExpression(IfE, Env);
  }

  const auto *ReturnS = dynamic_cast<const ast::ReturnStatement *>(Node);
  if (ReturnS) {
    auto Value = eval(ReturnS->ReturnValue.get(), Env);
    if (isError(Value)) {
      return Value;
    }

    return std::make_shared<object::ReturnValue>(std::move(Value));
  }

  const auto *LetS = dynamic_cast<const ast::LetStatement *>(Node);
  if (LetS) {
    auto Value = eval(LetS->Value.get(), Env);
    if (isError(Value)) {
      return Value;
    }

    Env.set(LetS->Name->Value, std::move(Value));
  }

  const auto *Identifier = dynamic_cast<const ast::Identifier *>(Node);
  if (Identifier) {
    return evalIdentifier(Identifier, Env);
  }

  auto *Function = dynamic_cast<ast::FunctionLiteral *>(Node);
  if (Function) {
    return std::make_shared<object::Function>(std::move(Function->Parameters),
                                              std::move(Function->Body), Env);
  }

  const auto *Call = dynamic_cast<const ast::CallExpression *>(Node);
  if (Call) {
    auto Function = eval(Call->Function.get(), Env);
    if (isError(Function)) {
      return Function;
    }

    auto Args = evalExpressions(Call->Arguments, Env);
    if (Args.size() == 1 && isError(Args.front())) {
      return Args.front();
    }

    return applyFunction(Function, Args);
  }

  const auto *String = dynamic_cast<const ast::String *>(Node);
  if (String) {
    return std::make_shared<object::String>(String->Value);
  }

  const auto *Array = dynamic_cast<const ast::ArrayLiteral *>(Node);
  if (Array) {
    auto Elements = evalExpressions(Array->Elements, Env);
    if (Elements.size() == 1 && isError(Elements.front())) {
      return Elements.front();
    }

    return std::make_shared<object::Array>(std::move(Elements));
  }

  const auto *IndexExp = dynamic_cast<const ast::IndexExpression *>(Node);
  if (IndexExp) {
    auto Left = eval(IndexExp->Left.get(), Env);
    if (isError(Left)) {
      return Left;
    }

    auto Index = eval(IndexExp->Index.get(), Env);
    if (isError(Index)) {
      return Index;
    }

    return evalIndexExpression(Left, Index);
  }

  return nullptr;
}

} // namespace monkey::evaluator
