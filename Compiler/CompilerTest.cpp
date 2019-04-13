#include "Compiler.h"

#include <Lexer/Lexer.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::compiler::test {

std::unique_ptr<ast::Program> parse(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  return P.parseProgram();
}

template <typename T> struct CompilerTestCase {
  const std::string Input;
  const std::vector<T> ExpectedConstants;
  const std::vector<code::Instructions> ExpectedInstructions;
};

code::Instructions
concatInstructions(const std::vector<code::Instructions> &Instructions) {
  code::Instructions Out;
  for (const auto &Ins : Instructions)
    std::copy(Ins.Value.begin(), Ins.Value.end(),
              std::back_inserter(Out.Value));

  return Out;
}

void testIntegerObject(int64_t Expected, const object::Object *Actual) {
  const auto *Integer = dynamic_cast<const object::Integer *>(Actual);
  ASSERT_THAT(Integer, testing::NotNull());
  ASSERT_EQ(Integer->Value, Expected);
}

void testInstructions(const std::vector<code::Instructions> &Expected,
                      const code::Instructions &Actual) {
  auto Concatted = concatInstructions(Expected);
  ASSERT_EQ(Actual.Value.size(), Concatted.Value.size());
  for (unsigned int Index = 0; Index < Actual.Value.size(); ++Index)
    ASSERT_EQ(Actual.Value.at(Index), Concatted.Value.at(Index));
}

template <typename T>
void testConstants(const std::vector<T> &Expected,
                   const std::vector<std::shared_ptr<object::Object>> &Actual) {
  ASSERT_EQ(Expected.size(), Actual.size());
  for (unsigned int Index = 0; Index < Expected.size(); ++Index) {
    if (std::is_same<int, T>())
      testIntegerObject(Expected.at(Index), Actual.at(Index).get());
  }
}

template <typename T>
void runCompilerTests(const std::vector<CompilerTestCase<T>> &Tests) {
  for (const auto &Test : Tests) {
    const auto Program = parse(Test.Input);

    Compiler C;
    const auto CompileError = C.compile(Program.get());
    ASSERT_TRUE(CompileError.empty());

    const auto ByteCode = C.byteCode();
    testInstructions(Test.ExpectedInstructions, ByteCode.Instructions);
    testConstants(Test.ExpectedConstants, ByteCode.Constants);
  }
}

TEST(CompilerTests, testIntegerArithmetic) {
  std::vector<CompilerTestCase<int>> Tests = {
      {"1 + 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1})}}};

  runCompilerTests(Tests);
}

} // namespace monkey::compiler::test