#include "VM.h"

#include <Object/BuiltIns.h>

#include <arpa/inet.h>
#include <cassert>

namespace monkey::vm {

namespace {

bool isTruthy(const object::Object *Obj) {
  const auto *BooleanObj = object::objCast<const object::Boolean *>(Obj);
  if (BooleanObj)
    return BooleanObj->Value;

  const auto *NullObj = object::objCast<const object::Null *>(Obj);
  if (NullObj)
    return false;

  return true;
}

} // namespace

VM::VM(compiler::ByteCode &&BC,
       std::array<std::shared_ptr<object::Object>, GlobalsSize> &Globals)
    : Constants(BC.Constants), Stack{nullptr}, SP(0), Globals(Globals),
      FrameIndex(1) {
  auto MainFn = std::make_shared<object::CompiledFunction>(
      std::move(BC.Instructions), 0, 0);
  auto MainClosure = std::make_shared<object::Closure>(std::move(MainFn));
  Frame MainFrame(std::move(MainClosure), 0);
  Frames.front() = std::move(MainFrame);
}

const object::Object *VM::lastPoppedStackElem() const {
  return Stack.at(SP).get();
}

void VM::run() {
  while (currentFrame().IP <
         (int)currentFrame().instructions().Value.size() - 1) {
    ++currentFrame().IP;

    auto &IP = currentFrame().IP;
    auto &Instructions = currentFrame().instructions();
    const auto Op(static_cast<code::OpCode>(Instructions.Value.at(IP)));

    switch (Op) {
    case code::OpCode::OpConstant: {
      const int16_t ConstIndex =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;

      push(Constants.at(ConstIndex));
      break;
    }
    case code::OpCode::OpAdd:
    case code::OpCode::OpSub:
    case code::OpCode::OpMul:
    case code::OpCode::OpDiv:
      executeBinaryOperation(Op);
      break;
    case code::OpCode::OpPop:
      pop();
      break;
    case code::OpCode::OpTrue:
      push(object::TrueGlobal);
      break;
    case code::OpCode::OpFalse:
      push(object::FalseGlobal);
      break;
    case code::OpCode::OpEqual:
    case code::OpCode::OpNotEqual:
    case code::OpCode::OpGreaterThan:
      executeComparison(Op);
      break;
    case code::OpCode::OpBang:
      executeBangOperator();
      break;
    case code::OpCode::OpMinus:
      executeMinusOperator();
      break;
    case code::OpCode::OpJump: {
      const int16_t JumpPos =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP = JumpPos - 1;
      break;
    }
    case code::OpCode::OpJumpNotTruthy: {
      const int16_t JumpPos =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;
      const auto &Condition = pop();
      if (!isTruthy(Condition.get()))
        IP = JumpPos - 1;
      break;
    }
    case code::OpCode::OpNull:
      push(object::NullGlobal);
      break;
    case code::OpCode::OpSetGlobal: {
      const int16_t GlobalIndex =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;
      Globals.at(GlobalIndex) = pop();
      break;
    }
    case code::OpCode::OpGetGlobal: {
      const int16_t GlobalIndex =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;
      push(Globals.at(GlobalIndex));
      break;
    }
    case code::OpCode::OpSetLocal: {
      const auto LocalIndex = Instructions.Value.at(IP + 1);
      auto &Frame = currentFrame();
      ++Frame.IP;
      Stack.at(Frame.BasePointer + LocalIndex) = pop();
      break;
    }
    case code::OpCode::OpGetLocal: {
      const auto LocalIndex = Instructions.Value.at(IP + 1);
      auto &Frame = currentFrame();
      ++Frame.IP;
      push(Stack.at(Frame.BasePointer + LocalIndex));
      break;
    }
    case code::OpCode::OpArray: {
      const int16_t NumElem =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;

      auto Array = buildArray(SP - NumElem, SP);
      SP = SP - NumElem;
      push(std::move(Array));
      break;
    }
    case code::OpCode::OpHash: {
      const int16_t NumElem =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      IP += 2;
      const auto Hash = buildHash(SP - NumElem, SP);
      SP -= NumElem;
      push(Hash);
      break;
    }
    case code::OpCode::OpIndex: {
      const auto &Index = pop();
      const auto &Left = pop();

      executeIndexExpression(Left, Index);
      break;
    }
    case code::OpCode::OpCall: {
      const auto NumArgs = Instructions.Value.at(IP + 1);
      ++currentFrame().IP;

      executeCall(NumArgs);
      break;
    }
    case code::OpCode::OpReturnValue: {
      auto Return = pop();
      const auto &Frame = popFrame();
      SP = Frame.BasePointer - 1;
      push(std::move(Return));
      break;
    }
    case code::OpCode::OpReturn: {
      const auto &Frame = popFrame();
      SP = Frame.BasePointer - 1;
      push(object::NullGlobal);
      break;
    }
    case code::OpCode::OpGetBuiltIn: {
      const auto BuiltInIndex = Instructions.Value.at(IP + 1);
      ++IP;

      const auto &Definition = object::BuiltIns.at(BuiltInIndex);
      push(Definition.second);
      break;
    }
    case code::OpCode::OpClosure: {
      const int16_t ConstIndex =
          ntohs(reinterpret_cast<int16_t &>(Instructions.Value.at(IP + 1)));
      const auto NumFree = Instructions.Value.at(IP + 3);
      IP += 3;
      pushClosure(ConstIndex, NumFree);
      break;
    }
    case code::OpCode::OpGetFree: {
      const auto FreeIndex = Instructions.Value.at(IP + 1);
      ++IP;

      const auto *CurrentClosure =
          object::objCast<const object::Closure *>(currentFrame().Cl.get());
      assert(CurrentClosure);
      push(CurrentClosure->Free.at(FreeIndex));
      break;
    }
    default:
      break;
    }
  }
}

const std::shared_ptr<object::Object> &VM::pop() {
  const auto &Obj = Stack.at(SP - 1);
  --SP;
  return Obj;
}

void VM::executeBinaryOperation(code::OpCode Op) {
  const auto &Right = pop();
  const auto &Left = pop();

  const auto LeftType = Left->type();
  const auto RightType = Right->type();

  if (LeftType == object::ObjectType::INTEGER_OBJ &&
      RightType == object::ObjectType::INTEGER_OBJ) {
    executeBinaryIntegerOperation(Op, Left, Right);
  } else if (LeftType == object::ObjectType::STRING_OBJ &&
             RightType == object::ObjectType::STRING_OBJ) {
    executeBinaryStringOperation(Op, Left, Right);
  } else {
    throw std::runtime_error(
        std::string("unsupported types for binary operation ") +
        object::objTypeToString(LeftType) + " " +
        object::objTypeToString(RightType));
  }
}

void VM::executeBinaryIntegerOperation(
    code::OpCode Op, const std::shared_ptr<object::Object> &Left,
    const std::shared_ptr<object::Object> &Right) {
  const auto LeftVal =
      object::objCast<const object::Integer *>(Left.get())->Value;
  const auto RightVal =
      object::objCast<const object::Integer *>(Right.get())->Value;

  const int64_t Result = [Op, LeftVal, RightVal]() {
    switch (Op) {
    case code::OpCode::OpAdd:
      return LeftVal + RightVal;
    case code::OpCode::OpSub:
      return LeftVal - RightVal;
    case code::OpCode::OpMul:
      return LeftVal * RightVal;
    case code::OpCode::OpDiv:
      return LeftVal / RightVal;
    default:
      throw std::runtime_error("unknown integer operator: " +
                               std::to_string(static_cast<unsigned char>(Op)));
    }
  }();

  push(std::make_shared<object::Integer>(Result));
}

void VM::executeBinaryStringOperation(
    code::OpCode Op, const std::shared_ptr<object::Object> &Left,
    const std::shared_ptr<object::Object> &Right) {
  if (Op != code::OpCode::OpAdd)
    throw std::runtime_error("unknown string operator: " +
                             std::to_string(static_cast<unsigned char>(Op)));

  const auto &LeftVal =
      object::objCast<const object::String *>(Left.get())->Value;
  const auto &RightVal =
      object::objCast<const object::String *>(Right.get())->Value;

  push(std::make_shared<object::String>(LeftVal + RightVal));
}

void VM::executeComparison(code::OpCode Op) {
  const auto &Right = pop();
  const auto &Left = pop();

  if (Left->type() == object::ObjectType::INTEGER_OBJ &&
      Right->type() == object::ObjectType::INTEGER_OBJ) {
    executeIntegerComparison(Op, Left, Right);
    return;
  }

  switch (Op) {
  case code::OpCode::OpEqual:
    push(object::nativeBooleanToBooleanObject(Right.get() == Left.get()));
    break;
  case code::OpCode::OpNotEqual:
    push(object::nativeBooleanToBooleanObject(Right.get() != Left.get()));
    break;
  default:
    throw std::runtime_error("unknown operator " +
                             std::to_string(static_cast<char>(Op)) + " (" +
                             object::objTypeToString(Left->type()) + " " +
                             object::objTypeToString(Right->type()) + ")");
  }
}

void VM::executeIntegerComparison(
    code::OpCode Op, const std::shared_ptr<object::Object> &Left,
    const std::shared_ptr<object::Object> &Right) {
  const auto LeftVal = static_cast<object::Integer *>(Left.get())->Value;
  const auto RightVal = static_cast<object::Integer *>(Right.get())->Value;

  switch (Op) {
  case code::OpCode::OpEqual:
    push(object::nativeBooleanToBooleanObject(LeftVal == RightVal));
    break;
  case code::OpCode::OpNotEqual:
    push(object::nativeBooleanToBooleanObject(LeftVal != RightVal));
    break;
  case code::OpCode::OpGreaterThan:
    push(object::nativeBooleanToBooleanObject(LeftVal > RightVal));
    break;
  default:
    throw std::runtime_error("unknown operator: " +
                             std::to_string(static_cast<char>(Op)));
  }
}

void VM::executeBangOperator() {
  const auto &Operand = pop();

  if (Operand.get() == object::TrueGlobal.get())
    push(object::FalseGlobal);
  else if (Operand.get() == object::FalseGlobal.get())
    push(object::TrueGlobal);
  else if (Operand.get() == object::NullGlobal.get())
    push(object::TrueGlobal);
  else
    push(object::FalseGlobal);
}

void VM::executeMinusOperator() {
  const auto &Operand = pop();

  if (Operand->type() != object::ObjectType::INTEGER_OBJ)
    throw std::runtime_error(std::string("unsupported type for negation: ") +
                             object::objTypeToString(Operand->type()));

  auto Value = static_cast<const object::Integer *>(Operand.get())->Value;
  push(std::make_shared<object::Integer>(-Value));
}

std::shared_ptr<object::Object> VM::buildArray(int StartIndex,
                                               int EndIndex) const {
  std::vector<std::shared_ptr<object::Object>> Elements(EndIndex - StartIndex,
                                                        nullptr);

  for (int I = StartIndex; I < EndIndex; ++I)
    Elements.at(I - StartIndex) = Stack.at(I);

  return std::make_shared<object::Array>(std::move(Elements));
}

std::shared_ptr<object::Object> VM::buildHash(int StartIndex,
                                              int EndIndex) const {
  std::unordered_map<object::HashKey, std::shared_ptr<object::Object>,
                     object::HashKeyHasher>
      HashedPairs;

  for (int I = StartIndex; I < EndIndex; I += 2) {
    const auto &Key = Stack.at(I);
    const auto &Value = Stack.at(I + 1);

    if (!object::hasHashKey(object::HashKey(Key)))
      throw std::runtime_error(std::string("unusable as hash key: ") +
                               object::objTypeToString(Key->type()));

    HashedPairs[object::HashKey(Key)] = Value;
  }

  return std::make_shared<object::Hash>(std::move(HashedPairs));
}

void VM::executeIndexExpression(const std::shared_ptr<object::Object> &Left,
                                const std::shared_ptr<object::Object> &Index) {
  if (Left->type() == object::ObjectType::ARRAY_OBJ &&
      Index->type() == object::ObjectType::INTEGER_OBJ)
    executeArrayIndex(Left, Index);
  else if (Left->type() == object::ObjectType::HASH_OBJ)
    executeHashIndex(Left, Index);
  else
    throw std::runtime_error(std::string("index operator not supported: ") +
                             object::objTypeToString(Left->type()));
}

void VM::executeArrayIndex(const std::shared_ptr<object::Object> &Array,
                           const std::shared_ptr<object::Object> &Index) {
  const auto *ArrayObj = object::objCast<const object::Array *>(Array.get());
  const auto I = object::objCast<const object::Integer *>(Index.get())->Value;
  const int Max = ArrayObj->Elements.size() - 1;

  if (I < 0 || I > Max) {
    push(object::NullGlobal);
    return;
  }

  push(ArrayObj->Elements.at(I));
}

void VM::executeHashIndex(const std::shared_ptr<object::Object> &Hash,
                          const std::shared_ptr<object::Object> &Index) {
  const auto *HashObj = object::objCast<const object::Hash *>(Hash.get());
  const object::HashKey Key(Index);

  if (!object::hasHashKey(Key))
    throw std::runtime_error(std::string("unusable as hash key: ") +
                             object::objTypeToString(Index->type()));

  const auto HashIter = HashObj->Pairs.find(Key);
  if (HashIter == HashObj->Pairs.end())
    push(object::NullGlobal);
  else
    push(HashIter->second);
}

Frame &VM::currentFrame() { return Frames.at(FrameIndex - 1); }

Frame &VM::popFrame() {
  auto &F = Frames.at(--FrameIndex);
  return F;
}

void VM::executeCall(int NumArgs) {
  const auto &Callee = Stack.at(SP - 1 - NumArgs);
  switch (Callee->type()) {
  case object::ObjectType::CLOSURE_OBJ:
    return callClosure(Callee, NumArgs);
  case object::ObjectType::BUILTIN_OBJ:
    return callBuiltIn(Callee, NumArgs);
  default:
    throw std::runtime_error("calling non-closure and non-built-in");
  }
}

void VM::callClosure(const std::shared_ptr<object::Object> &Cl, int NumArgs) {
  const auto *ClObj = object::objCast<const object::Closure *>(Cl.get());
  assert(ClObj);
  const auto *FnObj =
      object::objCast<const object::CompiledFunction *>(ClObj->Fn.get());
  if (NumArgs != FnObj->NumParameters)
    throw std::runtime_error("wrong number of arguments: want=" +
                             std::to_string(FnObj->NumParameters) +
                             ", got=" + std::to_string(NumArgs));
  pushFrame(Frame(Cl, SP - NumArgs));
  SP += FnObj->NumLocals + NumArgs;
}

void VM::callBuiltIn(const std::shared_ptr<object::Object> &Fn, int NumArgs) {
  std::vector<std::shared_ptr<object::Object>> Args;
  std::copy(Stack.begin() + SP - NumArgs, Stack.begin() + SP,
            std::back_inserter(Args));

  auto Result = object::objCast<const object::BuiltIn *>(Fn.get())->Fn(Args);
  SP = SP - NumArgs - 1;
  if (Result)
    push(std::move(Result));
  else
    push(object::NullGlobal);
}

void VM::pushClosure(int ConstIndex, int NumFree) {
  const auto &Constant = Constants.at(ConstIndex);
  const auto *Function =
      object::objCast<const object::CompiledFunction *>(Constant.get());
  if (!Function)
    throw std::runtime_error(std::string("not a function: ") +
                             object::objTypeToString(Constant->type()));

  std::vector<std::shared_ptr<object::Object>> Free;
  Free.reserve(NumFree);
  for (int I = 0; I < NumFree; ++I)
    Free.push_back(Stack.at(SP - NumFree + I));

  SP -= NumFree;
  push(std::make_unique<object::Closure>(Constant, std::move(Free)));
}

} // namespace monkey::vm
