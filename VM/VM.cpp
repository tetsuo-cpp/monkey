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
    code::OpCode Op(static_cast<code::OpCode>(Instructions.Value.at(IP)));
    switch (Op) {
    default:
      break;
    }
  }
}

} // namespace monkey::vm
