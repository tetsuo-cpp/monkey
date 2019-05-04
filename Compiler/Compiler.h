#pragma once

#include <AST/AST.h>
#include <Code/Code.h>
#include <Compiler/SymbolTable.h>
#include <Object/Object.h>

namespace monkey::compiler {

struct ByteCode {
  template <typename T>
  ByteCode(T &&Instructions,
           std::vector<std::shared_ptr<object::Object>> &Constants)
      : Instructions(std::forward<T>(Instructions)), Constants(Constants) {}

  code::Instructions Instructions;
  std::vector<std::shared_ptr<object::Object>> &Constants;
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
  Compiler(SymbolTable &, std::vector<std::shared_ptr<object::Object>> &);
  virtual ~Compiler() = default;

  void compile(const ast::Node *);
  ByteCode byteCode();

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
  std::vector<std::shared_ptr<object::Object>> &Constants;
  EmittedInstruction LastInstruction;
  EmittedInstruction PreviousInstruction;
  SymbolTable &SymTable;
};

} // namespace monkey::compiler
