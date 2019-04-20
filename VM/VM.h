#pragma once

#include <Compiler/Compiler.h>

namespace monkey::vm {

class VM {
public:
  explicit VM(compiler::ByteCode &&);
  virtual ~VM() = default;

  const object::Object *stackTop() const;
  void run();
  void push(const std::shared_ptr<object::Object> &);
  const std::shared_ptr<object::Object> &pop();

private:
  std::vector<std::shared_ptr<object::Object>> Constants;
  code::Instructions Instructions;
  std::vector<std::shared_ptr<object::Object>> Stack;
  unsigned int SP;
};

} // namespace monkey::vm
