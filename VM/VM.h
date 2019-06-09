#pragma once

#include "Frame.h"

#include <Compiler/Compiler.h>

#include <array>

static const size_t STACK_SIZE = 2048;
static const size_t GLOBALS_SIZE = 65536;
static const size_t MAX_FRAMES = 1024;

namespace monkey::vm {

class VM {
public:
  VM(compiler::ByteCode &&,
     std::array<std::shared_ptr<object::Object>, GLOBALS_SIZE> &);
  virtual ~VM() = default;

  const object::Object *lastPoppedStackElem() const;
  void run();

protected:
  template <typename T> void push(T &&Obj) {
    if (SP >= STACK_SIZE)
      throw std::runtime_error("stack overflow");

    Stack.at(SP++) = std::forward<T>(Obj);
  }
  virtual const std::shared_ptr<object::Object> &pop();
  void executeBinaryOperation(code::OpCode);
  void executeBinaryIntegerOperation(code::OpCode, const object::Object &,
                                     const object::Object &);
  void executeBinaryStringOperation(code::OpCode, const object::Object &,
                                    const object::Object &);
  void executeComparison(code::OpCode);
  void executeIntegerComparison(code::OpCode, const object::Object &,
                                const object::Object &);
  void executeBangOperator();
  void executeMinusOperator();
  std::shared_ptr<object::Object> buildArray(int, int) const;
  std::shared_ptr<object::Object> buildHash(int, int) const;
  void executeIndexExpression(const object::Object &,
                              const std::shared_ptr<object::Object> &);
  void executeArrayIndex(const object::Object &, const object::Object &);
  void executeHashIndex(const object::Object &,
                        const std::shared_ptr<object::Object> &);
  Frame &currentFrame();
  template <typename T> void pushFrame(T &&Frame) {
    Frames.at(FrameIndex++) = std::forward<T>(Frame);
  }
  Frame &popFrame();
  void executeCall(int);
  void callClosure(const std::shared_ptr<object::Object> &, int);
  void callBuiltIn(const object::Object &, int);
  void pushClosure(int, int);

  std::vector<std::shared_ptr<object::Object>> &Constants;
  std::array<std::shared_ptr<object::Object>, STACK_SIZE> Stack;
  unsigned int SP;
  std::array<std::shared_ptr<object::Object>, GLOBALS_SIZE> &Globals;
  std::array<Frame, MAX_FRAMES> Frames;
  int FrameIndex;
};

} // namespace monkey::vm
