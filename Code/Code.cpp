#include "Code.h"

#include <algorithm>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace monkey::code {

namespace {

std::vector<std::pair<OpCode, Definition>> Definitions = {
    {OpCode::OpConstant, {"OpConstant", {2}}},
    {OpCode::OpAdd, {"OpAdd", {}}},
    {OpCode::OpPop, {"OpPop", {}}},
    {OpCode::OpSub, {"OpSub", {}}},
    {OpCode::OpMul, {"OpMul", {}}},
    {OpCode::OpDiv, {"OpDiv", {}}},
    {OpCode::OpTrue, {"OpTrue", {}}},
    {OpCode::OpFalse, {"OpFalse", {}}},
    {OpCode::OpEqual, {"OpEqual", {}}},
    {OpCode::OpNotEqual, {"OpNotEqual", {}}},
    {OpCode::OpGreaterThan, {"OpGreaterThan", {}}},
    {OpCode::OpMinus, {"OpMinus", {}}},
    {OpCode::OpBang, {"OpBang", {}}},
    {OpCode::OpJumpNotTruthy, {"OpJumpNotTruthy", {2}}},
    {OpCode::OpJump, {"OpJump", {2}}},
    {OpCode::OpNull, {"OpNull", {}}},
    {OpCode::OpGetGlobal, {"OpGetGlobal", {2}}},
    {OpCode::OpSetGlobal, {"OpSetGlobal", {2}}}};

} // namespace

std::string Instructions::string() const {
  std::stringstream SS;
  unsigned int Index = 0;
  while (Index < Value.size()) {
    const Definition *Def;
    try {
      Def = &lookup(Value.at(Index));
    } catch (const std::runtime_error &E) {
      SS << "ERROR: " << E.what() << "\n";
      continue; // TODO: Does this make sense?
    }

    code::Instructions RemainingOps(
        std::vector<unsigned char>(Value.begin() + Index + 1, Value.end()));

    const auto Result = readOperands(*Def, RemainingOps);
    SS << std::setfill('0') << std::setw(4) << Index << " ";
    SS << fmtInstructions(*Def, Result.first) << "\n";

    Index += Result.second + 1;
  }

  return SS.str();
}

std::string
Instructions::fmtInstructions(const Definition &Def,
                              const std::vector<int> &Operands) const {
  const auto OperandCount = Def.OperandWidths.size();

  if (Operands.size() != OperandCount)
    return std::string("ERROR: operand len ") +
           std::to_string(Operands.size()) + " does not match defined " +
           std::to_string(OperandCount) + "\n";

  switch (OperandCount) {
  case 0:
    return Def.Name;
  case 1:
    return Def.Name + " " + std::to_string(Operands.front());
  }

  return std::string("ERROR: unhandled operandCount for ") + Def.Name + "\n";
}

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
      Operands.at(Index) = ntohs(Val);
      break;
    }

    Offset += Width;
  }

  return {Operands, Offset};
}

} // namespace monkey::code
