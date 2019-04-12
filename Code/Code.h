#pragma once

#include <string>
#include <vector>

namespace monkey::code {

using Instructions = std::vector<unsigned char>;

enum class OpCode : unsigned char { OpConstant };

struct Definition {
  std::string Name;
  std::vector<int> OperandWidths;
};

const Definition &lookup(unsigned char);

std::vector<unsigned char> make(OpCode, const std::vector<int> &);

} // namespace monkey::code
