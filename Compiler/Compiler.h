#pragma once

#include <AST/AST.h>
#include <Code/Code.h>
#include <Object/Object.h>

namespace monkey::compiler {

struct ByteCode {
  template <typename T0, typename T1>
  ByteCode(T0 &&Instructions, T1 &&Constants)
      : Instructions(std::forward<T0>(Instructions)),
        Constants(std::forward<T1>(Constants)) {}

  const code::Instructions Instructions;
  const std::vector<std::shared_ptr<object::Object>> Constants;
};

struct EmittedInstruction {
  EmittedInstruction() : Op(code::OpCode::OpPop), Position(0) {}
  EmittedInstruction(code::OpCode Op, unsigned int Position)
      : Op(Op), Position(Position) {}

  code::OpCode Op;
  unsigned int Position;
};

class Compiler {
public:
  Compiler() = default;
  virtual ~Compiler() = default;

  void compile(const ast::Node *);
  ByteCode byteCode() const;

private:
  template <typename T> int addConstant(T &&Obj) {
    Constants.push_back(std::forward<T>(Obj));
    return Constants.size() - 1;
  }

  int emit(code::OpCode, const std::vector<int> &);
  int addInstruction(const std::vector<unsigned char> &);
  void setLastInstruction(code::OpCode, unsigned int);
  bool lastInstructionIsPop() const;
  void removeLastPop();
  void replaceInstruction(unsigned int, std::vector<unsigned char> &);
  void changeOperand(unsigned int, int);

  code::Instructions Instructions;
  std::vector<std::shared_ptr<object::Object>> Constants;
  EmittedInstruction LastInstruction;
  EmittedInstruction PreviousInstruction;
};

} // namespace monkey::compiler
