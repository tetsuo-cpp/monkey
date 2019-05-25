#include "Evaluator.h"

#include <Object/BuiltIns.h>

#include <cassert>
#include <stdarg.h>

namespace monkey::evaluator {

namespace {

bool isError(const std::shared_ptr<object::Object> &Obj) {
  return Obj && Obj->type() == object::ObjectType::ERROR_OBJ;
}

std::shared_ptr<object::Object>
evalProgram(const std::vector<std::unique_ptr<ast::Statement>> &Statements,
            environment::Environment &Env) {
  std::shared_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get(), Env);
    auto *ReturnV = object::objCast<object::ReturnValue *>(Result.get());
    if (ReturnV)
      return std::move(ReturnV->Value);

    const auto *ErrorV = object::objCast<const object::Error *>(Result.get());
    if (ErrorV)
      return Result;
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
      if (Type == object::ObjectType::RETURN_VALUE_OBJ ||
          Type == object::ObjectType::ERROR_OBJ)
        return Result;
    }
  }

  return Result;
}

std::shared_ptr<object::Object>
evalBangOperatorExpression(const std::shared_ptr<object::Object> &Right) {
  const auto *Boolean = object::objCast<const object::Boolean *>(Right.get());
  if (Boolean)
    return std::make_shared<object::Boolean>(!Boolean->Value);

  const auto *Null = object::objCast<const object::Null *>(Right.get());
  if (Null)
    return std::make_shared<object::Boolean>(true);

  return std::make_shared<object::Boolean>(false);
}

std::shared_ptr<object::Object> evalMinusPrefixOperatorExpression(
    const std::shared_ptr<object::Object> &Right) {
  if (Right->type() != object::ObjectType::INTEGER_OBJ)
    return object::newError("unknown operator: -%s",
                            object::objTypeToString(Right->type()));

  const auto *Integer = object::objCast<const object::Integer *>(Right.get());
  if (!Integer)
    return std::make_shared<object::Null>();

  return std::make_shared<object::Integer>(-Integer->Value);
}

std::shared_ptr<object::Object>
evalPrefixExpression(const std::string &Operator,
                     const std::shared_ptr<object::Object> &Right) {
  if (Operator == "!")
    return evalBangOperatorExpression(Right);
  else if (Operator == "-")
    return evalMinusPrefixOperatorExpression(Right);
  else
    return object::newError("unknown operator: %s:%s", Operator.c_str(),
                            object::objTypeToString(Right->type()));
}

std::shared_ptr<object::Object>
evalIntegerInfixExpression(const std::string &Operator,
                           const std::shared_ptr<object::Object> &Left,
                           const std::shared_ptr<object::Object> &Right) {
  const auto *LeftInt = object::objCast<const object::Integer *>(Left.get());
  assert(LeftInt);
  const auto *RightInt = object::objCast<const object::Integer *>(Right.get());
  assert(RightInt);

  if (Operator == "+")
    return std::make_shared<object::Integer>(LeftInt->Value + RightInt->Value);
  else if (Operator == "-")
    return std::make_shared<object::Integer>(LeftInt->Value - RightInt->Value);
  else if (Operator == "*")
    return std::make_shared<object::Integer>(LeftInt->Value * RightInt->Value);
  else if (Operator == "/")
    return std::make_shared<object::Integer>(LeftInt->Value / RightInt->Value);
  else if (Operator == "<")
    return std::make_shared<object::Boolean>(LeftInt->Value < RightInt->Value);
  else if (Operator == ">")
    return std::make_shared<object::Boolean>(LeftInt->Value > RightInt->Value);
  else if (Operator == "==")
    return std::make_shared<object::Boolean>(LeftInt->Value == RightInt->Value);
  else if (Operator == "!=")
    return std::make_shared<object::Boolean>(LeftInt->Value != RightInt->Value);
  else
    return object::newError(
        "unknown operator: %s %s %s", object::objTypeToString(Left->type()),
        Operator.c_str(), object::objTypeToString(Right->type()));
}

std::shared_ptr<object::Object>
evalBooleanInfixExpression(const std::string &Operator,
                           const std::shared_ptr<object::Object> &Left,
                           const std::shared_ptr<object::Object> &Right) {
  const bool BothEqual = [&Left, &Right]() {
    if ((Left->type() == object::ObjectType::BOOLEAN_OBJ) ^
        (Right->type() == object::ObjectType::BOOLEAN_OBJ))
      return false;

    const auto *L = object::objCast<const object::Boolean *>(Left.get());
    const auto *R = object::objCast<const object::Boolean *>(Right.get());
    assert(L);
    assert(R);

    return L->Value == R->Value;
  }();

  if (Operator == "==")
    return std::make_shared<object::Boolean>(BothEqual);
  else if (Operator == "!=")
    return std::make_shared<object::Boolean>(!BothEqual);
  else {
    if ((Left->type() == object::ObjectType::BOOLEAN_OBJ) ^
        (Right->type() == object::ObjectType::BOOLEAN_OBJ))
      return object::newError(
          "type mismatch: %s %s %s", object::objTypeToString(Left->type()),
          Operator.c_str(), object::objTypeToString(Right->type()));
    else
      return object::newError(
          "unknown operator: %s %s %s", object::objTypeToString(Left->type()),
          Operator.c_str(), object::objTypeToString(Right->type()));
  }
}

std::shared_ptr<object::Object>
evalNullInfixExpression(const std::string &Operator,
                        const std::shared_ptr<object::Object> &Left,
                        const std::shared_ptr<object::Object> &Right) {
  const bool BothNull = Left->type() == object::ObjectType::NULL_OBJ &&
                        Right->type() == object::ObjectType::NULL_OBJ;

  if (Operator == "==")
    return std::make_shared<object::Boolean>(BothNull);
  else if (Operator == "!=")
    return std::make_shared<object::Boolean>(!BothNull);
  else
    return std::make_shared<object::Null>();
}

std::shared_ptr<object::Object>
evalStringInfixExpression(const std::string &Operator,
                          const std::shared_ptr<object::Object> &Left,
                          const std::shared_ptr<object::Object> &Right) {
  if (Operator != "+")
    return object::newError(
        "unknown operator: %s %s %s", object::objTypeToString(Left->type()),
        Operator.c_str(), object::objTypeToString(Right->type()));

  const auto *LeftS = object::objCast<const object::String *>(Left.get());
  const auto *RightS = object::objCast<const object::String *>(Right.get());
  assert(LeftS);
  assert(RightS);
  return std::make_shared<object::String>(LeftS->Value + RightS->Value);
}

std::shared_ptr<object::Object>
evalInfixExpression(const std::string &Operator,
                    const std::shared_ptr<object::Object> &Left,
                    const std::shared_ptr<object::Object> &Right) {
  if (Left->type() == object::ObjectType::INTEGER_OBJ &&
      Right->type() == object::ObjectType::INTEGER_OBJ)
    return evalIntegerInfixExpression(Operator, Left, Right);
  else if (Left->type() == object::ObjectType::NULL_OBJ ||
           Right->type() == object::ObjectType::NULL_OBJ)
    return evalNullInfixExpression(Operator, Left, Right);
  else if (Left->type() == object::ObjectType::BOOLEAN_OBJ ||
           Right->type() == object::ObjectType::BOOLEAN_OBJ)
    return evalBooleanInfixExpression(Operator, Left, Right);
  else if (Left->type() == object::ObjectType::STRING_OBJ ||
           Right->type() == object::ObjectType::STRING_OBJ)
    return evalStringInfixExpression(Operator, Left, Right);
  else if (Left->type() != Right->type())
    return object::newError(
        "type mismatch: %s %s %s", object::objTypeToString(Left->type()),
        Operator.c_str(), object::objTypeToString(Right->type()));
  else
    return object::newError(
        "unknown operator: %s %s %s", object::objTypeToString(Left->type()),
        Operator.c_str(), object::objTypeToString(Right->type()));
}

bool isTruthy(const object::Object *Obj) {
  const auto *NullObj = object::objCast<const object::Null *>(Obj);
  if (NullObj)
    return false;

  const auto *BooleanObj = object::objCast<const object::Boolean *>(Obj);
  if (BooleanObj)
    return BooleanObj->Value;

  return true;
}

std::shared_ptr<object::Object>
evalIfExpression(const ast::IfExpression *Node, environment::Environment &Env) {
  auto Cond = eval(Node->Condition.get(), Env);
  if (isError(Cond))
    return Cond;

  if (isTruthy(Cond.get()))
    return eval(Node->Consequence.get(), Env);
  else if (Node->Alternative)
    return eval(Node->Alternative.get(), Env);
  else
    return std::make_shared<object::Null>();
}

std::shared_ptr<object::Object>
evalIdentifier(const ast::Identifier *Identifier,
               environment::Environment &Env) {
  const auto &Value = Env.get(Identifier->Value);
  if (Value)
    return Value;

  const auto BIter = std::find_if(
      object::BuiltIns.begin(), object::BuiltIns.end(),
      [Identifier](
          const std::pair<std::string, std::shared_ptr<object::BuiltIn>> &Fn) {
        return Fn.first == Identifier->Value;
      });

  if (BIter != object::BuiltIns.end())
    return BIter->second;

  return object::newError("identifier not found: %s",
                          Identifier->Value.c_str());
}

std::vector<std::shared_ptr<object::Object>>
evalExpressions(const std::vector<std::unique_ptr<ast::Expression>> &Arguments,
                environment::Environment &Env) {
  std::vector<std::shared_ptr<object::Object>> Results;

  for (auto &Arg : Arguments) {
    auto Evaluated = eval(Arg.get(), Env);
    if (isError(Evaluated))
      return {Evaluated};

    Results.push_back(std::move(Evaluated));
  }

  return Results;
}

std::shared_ptr<object::Object>
evalArrayIndexExpression(const std::shared_ptr<object::Object> &Array,
                         const std::shared_ptr<object::Object> &Index) {
  const auto *ArrayObj = object::objCast<const object::Array *>(Array.get());
  assert(ArrayObj);
  const auto *Idx = object::objCast<const object::Integer *>(Index.get());
  assert(Idx);

  if (Idx->Value < 0 ||
      Idx->Value >= static_cast<int64_t>(ArrayObj->Elements.size()))
    return std::make_shared<object::Null>();

  return ArrayObj->Elements.at(Idx->Value);
}

std::shared_ptr<object::Object>
evalHashIndexExpression(const std::shared_ptr<object::Object> &Hash,
                        const std::shared_ptr<object::Object> &Index) {
  const auto *HashObj = object::objCast<const object::Hash *>(Hash.get());
  assert(HashObj);

  const object::HashKey HK(Index);
  if (!object::hasHashKey(HK))
    return object::newError("unusable as hash key: %s",
                            object::objTypeToString(Index->type()));

  const auto Iter = HashObj->Pairs.find(HK);
  if (Iter == HashObj->Pairs.end())
    return std::make_shared<object::Null>();

  return Iter->second;
}

std::shared_ptr<object::Object>
evalIndexExpression(const std::shared_ptr<object::Object> &Left,
                    const std::shared_ptr<object::Object> &Index) {
  if (Left->type() == object::ObjectType::ARRAY_OBJ &&
      Index->type() == object::ObjectType::INTEGER_OBJ)
    return evalArrayIndexExpression(Left, Index);
  else if (Left->type() == object::ObjectType::HASH_OBJ)
    return evalHashIndexExpression(Left, Index);

  return object::newError("index operator not supported: %s",
                          object::objTypeToString(Left->type()));
}

std::shared_ptr<object::Object> evalHashLiteral(const ast::HashLiteral *Hash,
                                                environment::Environment &Env) {
  std::unordered_map<object::HashKey, std::shared_ptr<object::Object>,
                     object::HashKeyHasher>
      Pairs;

  for (const auto &P : Hash->Pairs) {
    auto Key = eval(P.first.get(), Env);
    if (isError(Key))
      return Key;

    object::HashKey HK(Key);
    if (!object::hasHashKey(HK))
      return object::newError("unusable as hash key: %s",
                              object::objTypeToString(Key->type()));

    auto Value = eval(P.second.get(), Env);
    if (isError(Value))
      return Value;

    Pairs.emplace(std::move(HK), Value);
  }

  return std::make_shared<object::Hash>(std::move(Pairs));
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
  const auto *ReturnValue = object::objCast<object::ReturnValue *>(Obj.get());
  if (ReturnValue)
    return ReturnValue->Value;

  return Obj;
}

std::shared_ptr<object::Object>
applyFunction(const std::shared_ptr<object::Object> &Fn,
              const std::vector<std::shared_ptr<object::Object>> &Args) {
  const auto *Function = object::objCast<const object::Function *>(Fn.get());
  if (Function) {
    auto ExtendedEnv = extendFunctionEnv(Function, Args);
    auto Evaluated = eval(Function->Body.get(), ExtendedEnv);
    return unwrapReturnValue(Evaluated);
  }

  const auto *BuiltIn = object::objCast<const object::BuiltIn *>(Fn.get());
  if (BuiltIn) {
    auto Result = BuiltIn->Fn(Args);
    if (Result)
      return Result;
    else
      return std::make_shared<object::Null>();
  }

  return object::newError("not a function %s",
                          object::objTypeToString(Fn->type()));
}

} // namespace

// TODO: Maybe switch to using a visitor to avoid the constant dynamic casting?
std::shared_ptr<object::Object> eval(ast::Node *Node,
                                     environment::Environment &Env) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program)
    return evalProgram(Program->Statements, Env);

  const auto *ExprS = dynamic_cast<const ast::ExpressionStatement *>(Node);
  if (ExprS)
    return eval(ExprS->Expr.get(), Env);

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL)
    return std::make_shared<object::Integer>(IntegerL->Value);

  const auto *BooleanL = dynamic_cast<const ast::Boolean *>(Node);
  if (BooleanL)
    return std::make_shared<object::Boolean>(BooleanL->Value);

  const auto *PrefixE = dynamic_cast<const ast::PrefixExpression *>(Node);
  if (PrefixE) {
    auto Right = eval(PrefixE->Right.get(), Env);
    if (isError(Right))
      return Right;

    return evalPrefixExpression(PrefixE->Operator, Right);
  }

  const auto *InfixE = dynamic_cast<const ast::InfixExpression *>(Node);
  if (InfixE) {
    auto Left = eval(InfixE->Left.get(), Env);
    if (isError(Left))
      return Left;

    auto Right = eval(InfixE->Right.get(), Env);
    if (isError(Right))
      return Right;

    return evalInfixExpression(InfixE->Operator, Left, Right);
  }

  const auto *BlockS = dynamic_cast<const ast::BlockStatement *>(Node);
  if (BlockS)
    return evalBlockStatement(BlockS->Statements, Env);

  const auto *IfE = dynamic_cast<const ast::IfExpression *>(Node);
  if (IfE)
    return evalIfExpression(IfE, Env);

  const auto *ReturnS = dynamic_cast<const ast::ReturnStatement *>(Node);
  if (ReturnS) {
    auto Value = eval(ReturnS->ReturnValue.get(), Env);
    if (isError(Value))
      return Value;

    return std::make_shared<object::ReturnValue>(std::move(Value));
  }

  const auto *LetS = dynamic_cast<const ast::LetStatement *>(Node);
  if (LetS) {
    auto Value = eval(LetS->Value.get(), Env);
    if (isError(Value))
      return Value;

    Env.set(LetS->Name->Value, std::move(Value));
  }

  const auto *Identifier = dynamic_cast<const ast::Identifier *>(Node);
  if (Identifier)
    return evalIdentifier(Identifier, Env);

  auto *Function = dynamic_cast<ast::FunctionLiteral *>(Node);
  if (Function)
    return std::make_shared<object::Function>(std::move(Function->Parameters),
                                              std::move(Function->Body), Env);

  const auto *Call = dynamic_cast<const ast::CallExpression *>(Node);
  if (Call) {
    auto CallFunc = eval(Call->Function.get(), Env);
    if (isError(CallFunc))
      return CallFunc;

    auto Args = evalExpressions(Call->Arguments, Env);
    if (Args.size() == 1 && isError(Args.front()))
      return Args.front();

    return applyFunction(CallFunc, Args);
  }

  const auto *String = dynamic_cast<const ast::String *>(Node);
  if (String)
    return std::make_shared<object::String>(String->Value);

  const auto *Array = dynamic_cast<const ast::ArrayLiteral *>(Node);
  if (Array) {
    auto Elements = evalExpressions(Array->Elements, Env);
    if (Elements.size() == 1 && isError(Elements.front()))
      return Elements.front();

    return std::make_shared<object::Array>(std::move(Elements));
  }

  const auto *IndexExp = dynamic_cast<const ast::IndexExpression *>(Node);
  if (IndexExp) {
    auto Left = eval(IndexExp->Left.get(), Env);
    if (isError(Left))
      return Left;

    auto Index = eval(IndexExp->Index.get(), Env);
    if (isError(Index))
      return Index;

    return evalIndexExpression(Left, Index);
  }

  const auto *Hash = dynamic_cast<const ast::HashLiteral *>(Node);
  if (Hash)
    return evalHashLiteral(Hash, Env);

  return nullptr;
}

} // namespace monkey::evaluator
