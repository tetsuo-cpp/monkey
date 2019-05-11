#pragma once

#include <Compiler/Compiler.h>

#include <array>

namespace {

const size_t StackSize = 2048;
const size_t GlobalsSize = 65536;

} // namespace

namespace monkey::vm {

class VM {
public:
  VM(compiler::ByteCode &&,
     std::array<std::shared_ptr<object::Object>, GlobalsSize> &);
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
  void executeBinaryStringOperation(code::OpCode,
                                    const std::shared_ptr<object::Object> &,
                                    const std::shared_ptr<object::Object> &);
  void executeComparison(code::OpCode);
  void executeIntegerComparison(code::OpCode,
                                const std::shared_ptr<object::Object> &,
                                const std::shared_ptr<object::Object> &);
  void executeBangOperator();
  void executeMinusOperator();
  std::shared_ptr<object::Object> buildArray(int, int) const;
  std::shared_ptr<object::Object> buildHash(int, int) const;
  void executeIndexExpression(const std::shared_ptr<object::Object> &,
                              const std::shared_ptr<object::Object> &);
  void executeArrayIndex(const std::shared_ptr<object::Object> &,
                         const std::shared_ptr<object::Object> &);
  void executeHashIndex(const std::shared_ptr<object::Object> &,
                        const std::shared_ptr<object::Object> &);

  std::vector<std::shared_ptr<object::Object>> &Constants;
  code::Instructions Instructions;
  std::array<std::shared_ptr<object::Object>, StackSize> Stack;
  unsigned int SP;
  std::array<std::shared_ptr<object::Object>, GlobalsSize> &Globals;
};

} // namespace monkey::vm
