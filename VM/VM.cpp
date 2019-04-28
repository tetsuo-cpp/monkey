#include "VM.h"

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

VM::VM(compiler::ByteCode &&BC)
    : Constants(std::move(BC.Constants)),
      Instructions(std::move(BC.Instructions)), Stack{nullptr}, SP(0) {}

const object::Object *VM::lastPoppedStackElem() const {
  return Stack.at(SP).get();
}

void VM::run() {
  for (unsigned int IP = 0; IP < Instructions.Value.size(); ++IP) {
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
    return;
  }

  throw std::runtime_error(
      std::string("unsupported types for binary operation ") +
      object::objTypeToString(LeftType) + " " +
      object::objTypeToString(RightType));
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

} // namespace monkey::vm
