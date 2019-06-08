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
    {OpCode::OpSetGlobal, {"OpSetGlobal", {2}}},
    {OpCode::OpArray, {"OpArray", {2}}},
    {OpCode::OpHash, {"OpHash", {2}}},
    {OpCode::OpIndex, {"OpIndex", {}}},
    {OpCode::OpCall, {"OpCall", {1}}},
    {OpCode::OpReturnValue, {"OpReturnValue", {}}},
    {OpCode::OpReturn, {"OpReturn", {}}},
    {OpCode::OpGetLocal, {"OpGetLocal", {1}}},
    {OpCode::OpSetLocal, {"OpSetLocal", {1}}},
    {OpCode::OpGetBuiltIn, {"OpGetBuiltIn", {1}}},
    {OpCode::OpClosure, {"OpClosure", {2, 1}}},
    {OpCode::OpGetFree, {"OpGetFree", {1}}}};

} // namespace

std::string Instructions::string() const {
  std::stringstream SS;
  unsigned int I = 0;
  while (I < Value.size()) {
    const Definition *Def;
    try {
      Def = &lookup(Value.at(I));
    } catch (const std::runtime_error &E) {
      SS << "ERROR: " << E.what() << "\n";
      continue;
    }

    code::Instructions RemainingOps(
        std::vector<char>(Value.begin() + I + 1, Value.end()));

    const auto Result = readOperands(*Def, RemainingOps);
    SS << std::setfill('0') << std::setw(4) << I << " ";
    SS << fmtInstructions(*Def, Result.first) << "\n";

    I += Result.second + 1;
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
  case 2:
    return Def.Name + " " + std::to_string(Operands.front()) + " " +
           std::to_string(Operands.at(1));
  }

  return std::string("ERROR: unhandled operandCount for ") + Def.Name + "\n";
}

const Definition &lookup(char Op) {
  const auto Iter =
      std::find_if(Definitions.begin(), Definitions.end(),
                   [Op](const std::pair<OpCode, Definition> &Def) {
                     return Def.first == static_cast<OpCode>(Op);
                   });

  if (Iter == Definitions.end())
    throw std::runtime_error("opcode " + std::string(1, Op) + " undefined");

  return Iter->second;
}

std::vector<char> make(OpCode Op, const std::vector<int> &Operands) {
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

  std::vector<char> Instruction(InstructionLen, 0);
  Instruction.front() = static_cast<char>(Op);

  unsigned int Offset = 1;
  for (unsigned int I = 0; I < Operands.size(); ++I) {
    const auto Width = Iter->second.OperandWidths.at(I);
    switch (Width) {
    case 1:
      Instruction.at(Offset) = Operands.at(I);
      break;
    case 2:
      int16_t &WritePos = reinterpret_cast<int16_t &>(Instruction.at(Offset));
      WritePos = htons(Operands.at(I));
      break;
    }

    Offset += Width;
  }

  return Instruction;
}

std::pair<std::vector<int>, int> readOperands(const Definition &Def,
                                              const Instructions &Ins) {
  std::vector<int> Operands(Def.OperandWidths.size(), 0);
  int Offset = 0;

  for (unsigned int I = 0; I < Operands.size(); ++I) {
    const auto Width = Def.OperandWidths.at(I);
    switch (Width) {
    case 2: {
      const auto Val = reinterpret_cast<const uint16_t &>(Ins.Value.at(Offset));
      Operands.at(I) = ntohs(Val);
      break;
    }
    case 1:
      Operands.at(I) = reinterpret_cast<const uint8_t &>(Ins.Value.at(Offset));
      break;
    }

    Offset += Width;
  }

  return {Operands, Offset};
}

} // namespace monkey::code
