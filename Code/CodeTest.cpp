#include "Code.h"

#include <gtest/gtest.h>

namespace monkey::code::test {

TEST(CodeTests, testMake) {
  const std::vector<
      std::tuple<OpCode, std::vector<int>, std::vector<unsigned char>>>
      Tests = {{OpCode::OpConstant,
                {65534},
                {static_cast<char>(OpCode::OpConstant), 255, 254}}};

  for (const auto &Test : Tests) {
    const auto &Op = std::get<0>(Test);
    const auto &Operands = std::get<1>(Test);
    const auto &Expected = std::get<2>(Test);
    const auto Instruction = make(Op, Operands);

    ASSERT_EQ(Instruction.size(), Expected.size());

    for (unsigned int Index = 0; Index < Instruction.size(); ++Index)
      ASSERT_EQ(Instruction.at(Index), Expected.at(Index));
  }
}

TEST(CodeTests, testInstructionString) {
  const std::vector<Instructions> Ins = {make(OpCode::OpConstant, {1}),
                                         make(OpCode::OpConstant, {2}),
                                         make(OpCode::OpConstant, {65535})};

  const std::string Expected("0000 OpConstant 1"
                             "0003 OpConstant 2"
                             "0006 OpConstant 65535");

  Instructions Concatted;
  for (const auto &I : Ins)
    std::copy(I.Value.begin(), I.Value.end(),
              std::back_inserter(Concatted.Value));

  EXPECT_EQ(Concatted.string(), Expected);
}

TEST(CodeTests, testReadOperands) {
  const std::vector<std::tuple<OpCode, std::vector<int>, int>> Tests = {
      {OpCode::OpConstant, {65535}, 2}};

  for (const auto &Test : Tests) {
    const auto Op = std::get<0>(Test);
    const auto &Operands = std::get<1>(Test);
    const auto BytesRead = std::get<2>(Test);

    const auto Instruction = make(Op, Operands);
    const auto &Def = lookup(static_cast<unsigned char>(Op));

    // Get the non-opcode part.
    Instructions OperandsPart;
    std::copy(Instruction.begin() + 1, Instruction.end(),
              std::back_inserter(OperandsPart.Value));

    const auto Value = readOperands(Def, OperandsPart.Value);
    EXPECT_EQ(Value.second, BytesRead);

    for (unsigned int Index = 0; Index < Operands.size(); ++Index)
      EXPECT_EQ(Operands.at(Index), Value.first.at(Index));
  }
}

} // namespace monkey::code::test
