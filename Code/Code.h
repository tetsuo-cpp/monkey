#pragma once

#include <string>
#include <vector>

namespace monkey::code {

struct Definition;

struct Instructions {
  Instructions() = default;
  Instructions(const std::vector<char> &Value) : Value(Value) {}
  Instructions(std::vector<char> &&Value) : Value(std::move(Value)) {}

  std::string string() const;
  std::string fmtInstructions(const Definition &,
                              const std::vector<int> &) const;

  std::vector<char> Value;
};

enum class OpCode : char {
  OpConstant,
  OpAdd,
  OpPop,
  OpSub,
  OpMul,
  OpDiv,
  OpTrue,
  OpFalse,
  OpEqual,
  OpNotEqual,
  OpGreaterThan,
  OpMinus,
  OpBang,
  OpJumpNotTruthy,
  OpJump,
  OpNull,
  OpGetGlobal,
  OpSetGlobal,
  OpArray,
  OpHash,
  OpIndex,
  OpCall,
  OpReturnValue,
  OpReturn,
  OpGetLocal,
  OpSetLocal,
  OpGetBuiltIn,
  OpClosure,
  OpGetFree
};

struct Definition {
  std::string Name;
  std::vector<int> OperandWidths;
};

const Definition &lookup(char);
std::vector<char> make(OpCode, const std::vector<int> &);
std::pair<std::vector<int>, int> readOperands(const Definition &,
                                              const Instructions &);

} // namespace monkey::code
