#include "Code.h"

#include <algorithm>
#include <arpa/inet.h>
#include <stdexcept>

namespace monkey::code {

namespace {

std::vector<std::pair<OpCode, Definition>> Definitions = {
    {OpCode::OpConstant, {"OpConstant", {2}}}};

} // namespace

std::string Instructions::string() const { return std::string(); }

const Definition &lookup(unsigned char Op) {
  const auto Iter =
      std::find_if(Definitions.begin(), Definitions.end(),
                   [Op](const std::pair<OpCode, Definition> &Def) {
                     return Def.first == static_cast<OpCode>(Op);
                   });

  if (Iter == Definitions.end())
    throw std::runtime_error("opcode " + std::string(1, Op) + " undefined");

  return Iter->second;
}

std::vector<unsigned char> make(OpCode Op, const std::vector<int> &Operands) {
  const auto Iter =
      std::find_if(Definitions.begin(), Definitions.end(),
                   [Op](const std::pair<OpCode, Definition> &Def) {
                     return Def.first == Op;
                   });

  if (Iter == Definitions.end())
    return {};

  unsigned int InstructionLen = 1;
  for (const auto W : Iter->second.OperandWidths)
    InstructionLen += W;

  std::vector<unsigned char> Instruction(InstructionLen, 0);
  Instruction.front() = static_cast<unsigned char>(Op);

  unsigned Offset = 1;
  for (unsigned int Index = 0; Index < Operands.size(); ++Index) {
    const auto Width = Iter->second.OperandWidths.at(Index);
    switch (Width) {
    case 2:
      int16_t &WritePos = reinterpret_cast<int16_t &>(Instruction.at(Offset));
      WritePos = htons(Operands.at(Index));
      break;
    }

    Offset += Width;
  }

  return Instruction;
}

std::pair<std::vector<int>, int> readOperands(const Definition &Def,
                                              const Instructions &Ins) {
  std::vector<int> Operands(Def.OperandWidths.size(), 0);
  unsigned int Offset = 0;

  for (unsigned int Index = 0; Index < Operands.size(); ++Index) {
    const auto Width = Def.OperandWidths.at(Index);
    switch (Width) {
    case 2:
      uint16_t Val = reinterpret_cast<const uint16_t &>(Ins.Value.at(Offset));
      Val = htons(Val);
      Operands.at(Index) = Val;
      break;
    }

    Offset += Width;
  }

  return {Operands, Offset};
}

} // namespace monkey::code
