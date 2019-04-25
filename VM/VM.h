#pragma once

#include <Compiler/Compiler.h>

#include <array>

namespace {

const size_t StackSize = 2048;

} // namespace

namespace monkey::vm {

class VM {
public:
  explicit VM(compiler::ByteCode &&);
  virtual ~VM() = default;

  const object::Object *lastPoppedStackElem() const;
  void run();
  void push(const std::shared_ptr<object::Object> &);
  const std::shared_ptr<object::Object> &pop();

private:
  void executeBinaryOperation(code::OpCode);
  void executeBinaryIntegerOperation(code::OpCode,
                                     const std::shared_ptr<object::Object> &,
                                     const std::shared_ptr<object::Object> &);
  void executeComparison(code::OpCode);
  void executeIntegerComparison(code::OpCode,
                                const std::shared_ptr<object::Object> &,
                                const std::shared_ptr<object::Object> &);

  std::vector<std::shared_ptr<object::Object>> Constants;
  code::Instructions Instructions;
  std::array<std::shared_ptr<object::Object>, StackSize> Stack;
  unsigned int SP;
};

} // namespace monkey::vm
