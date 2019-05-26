#pragma once

#include "Frame.h"

#include <Compiler/Compiler.h>

#include <array>

static const size_t StackSize = 2048;
static const size_t GlobalsSize = 65536;
static const size_t MaxFrames = 1024;

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
  Frame &currentFrame();
  template <typename T> void pushFrame(T &&Frame) {
    Frames.at(FrameIndex++) = std::forward<T>(Frame);
  }
  Frame &popFrame();
  void executeCall(int);
  void callClosure(const std::shared_ptr<object::Object> &, int);
  void callBuiltIn(const std::shared_ptr<object::Object> &, int);
  void pushClosure(int, int);

  std::vector<std::shared_ptr<object::Object>> &Constants;
  std::array<std::shared_ptr<object::Object>, StackSize> Stack;
  unsigned int SP;
  std::array<std::shared_ptr<object::Object>, GlobalsSize> &Globals;
  std::array<Frame, MaxFrames> Frames;
  int FrameIndex;
};

} // namespace monkey::vm
