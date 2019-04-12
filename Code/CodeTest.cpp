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
    auto Instruction = make(Op, Operands);

    ASSERT_EQ(Instruction.size(), Expected.size());

    for (unsigned int Index = 0; Index < Instruction.size(); ++Index)
      ASSERT_EQ(Instruction.at(Index), Expected.at(Index));
  }
}

} // namespace monkey::code::test
