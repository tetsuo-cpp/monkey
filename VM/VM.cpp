#include "VM.h"

namespace monkey::vm {

namespace {

const size_t StackSize = 2048;

} // namespace

VM::VM(compiler::ByteCode &&BC)
    : Constants(std::move(BC.Constants)),
      Instructions(std::move(BC.Instructions)), Stack(StackSize, nullptr),
      SP(0) {}

const object::Object *VM::stackTop() const {
  if (SP == 0)
    return nullptr;

  return Stack.at(SP - 1).get();
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
    case code::OpCode::OpAdd: {
      const auto &Right = pop();
      const auto &Left = pop();
      const auto LeftValue =
          dynamic_cast<const object::Integer *>(Left.get())->Value;
      const auto RightValue =
          dynamic_cast<const object::Integer *>(Right.get())->Value;

      const auto Result = LeftValue + RightValue;
      push(std::make_shared<object::Integer>(Result));
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

} // namespace monkey::vm
