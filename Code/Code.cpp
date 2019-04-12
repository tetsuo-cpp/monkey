#include "Code.h"

namespace monkey::code {

std::vector<std::pair<OpCode, Definition>> Definitions = {
    {OpCode::OpConstant, {"OpConstant", {2}}}};

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
    auto Width = Iter->second.OperandWidths.at(Index);
    switch (Width) {
    case 2:
      const auto Value = htons(Operands.at(Index));
      Instruction.at(Offset) = Value >> 8;
      Instruction.at(Offset + 1) = Value & 0x0F;
      break;
    }

    Offset += Width;
  }

  return Instruction;
}

} // namespace monkey::code
