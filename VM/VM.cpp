#include "VM.h"

#include <arpa/inet.h>

namespace monkey::vm {

namespace {

static const std::shared_ptr<object::Object> TrueGlobal =
    std::make_shared<object::Boolean>(true);

static const std::shared_ptr<object::Object> FalseGlobal =
    std::make_shared<object::Boolean>(false);

static const std::shared_ptr<object::Object> NullGlobal =
    std::make_shared<object::Null>();

const std::shared_ptr<object::Object> nativeBooleanToBooleanObject(bool Val) {
  if (Val)
    return TrueGlobal;

  return FalseGlobal;
}

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
  auto MainFn =
      std::make_shared<object::CompiledFunction>(std::move(BC.Instructions), 0);
  Frame MainFrame(std::move(MainFn), 0);
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
      push(TrueGlobal);
      break;
    case code::OpCode::OpFalse:
      push(FalseGlobal);
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
      push(NullGlobal);
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
      const int8_t LocalIndex = Instructions.Value.at(IP + 1);
      auto &Frame = currentFrame();
      ++Frame.IP;
      Stack.at(Frame.BasePointer + LocalIndex) = pop();
      break;
    }
    case code::OpCode::OpGetLocal: {
      const int8_t LocalIndex = Instructions.Value.at(IP + 1);
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
      const auto &Fn = Stack.at(SP - 1);
      const auto *FnObj =
          object::objCast<const object::CompiledFunction *>(Fn.get());
      if (!FnObj)
        throw std::runtime_error("calling non-function");
      pushFrame(Frame(Fn, SP));
      SP += FnObj->NumLocals;
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
      push(NullGlobal);
      break;
    }
    default:
      break;
    }
  }
}

void VM::push(const std::shared_ptr<object::Object> &Obj) {
  if (SP >= StackSize)
    throw std::runtime_error("stack overflow");

  Stack.at(SP++) = Obj;
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
    push(nativeBooleanToBooleanObject(Right.get() == Left.get()));
    break;
  case code::OpCode::OpNotEqual:
    push(nativeBooleanToBooleanObject(Right.get() != Left.get()));
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
    push(nativeBooleanToBooleanObject(LeftVal == RightVal));
    break;
  case code::OpCode::OpNotEqual:
    push(nativeBooleanToBooleanObject(LeftVal != RightVal));
    break;
  case code::OpCode::OpGreaterThan:
    push(nativeBooleanToBooleanObject(LeftVal > RightVal));
    break;
  default:
    throw std::runtime_error("unknown operator: " +
                             std::to_string(static_cast<char>(Op)));
  }
}

void VM::executeBangOperator() {
  const auto &Operand = pop();

  if (Operand.get() == TrueGlobal.get())
    push(FalseGlobal);
  else if (Operand.get() == FalseGlobal.get())
    push(TrueGlobal);
  else if (Operand.get() == NullGlobal.get())
    push(TrueGlobal);
  else
    push(FalseGlobal);
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
    push(NullGlobal);
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
    push(NullGlobal);
  else
    push(HashIter->second);
}

Frame &VM::currentFrame() { return Frames.at(FrameIndex - 1); }

Frame &VM::popFrame() {
  auto &F = Frames.at(FrameIndex--);
  return F;
}

} // namespace monkey::vm
