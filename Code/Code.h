#pragma once

#include <string>
#include <vector>

namespace monkey::code {

struct Definition;

struct Instructions {
  Instructions() = default;
  Instructions(const std::vector<unsigned char> &Value) : Value(Value) {}
  Instructions(std::vector<unsigned char> &&Value) : Value(std::move(Value)) {}

  std::string string() const;
  std::string fmtInstructions(const Definition &,
                              const std::vector<int> &) const;

  std::vector<unsigned char> Value;
};

enum class OpCode : unsigned char {
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
  OpSetGlobal
};

struct Definition {
  std::string Name;
  std::vector<int> OperandWidths;
};

const Definition &lookup(unsigned char);

std::vector<unsigned char> make(OpCode, const std::vector<int> &);

std::pair<std::vector<int>, int> readOperands(const Definition &,
                                              const Instructions &);

} // namespace monkey::code
