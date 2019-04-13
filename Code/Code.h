#pragma once

#include <string>
#include <vector>

namespace monkey::code {

struct Instructions {
  Instructions() = default;
  Instructions(const std::vector<unsigned char> &Value) : Value(Value) {}
  Instructions(std::vector<unsigned char> &&Value) : Value(std::move(Value)) {}

  std::string string() const;

  std::vector<unsigned char> Value;
};

enum class OpCode : unsigned char { OpConstant };

struct Definition {
  std::string Name;
  std::vector<int> OperandWidths;
};

const Definition &lookup(unsigned char);

std::vector<unsigned char> make(OpCode, const std::vector<int> &);

} // namespace monkey::code
