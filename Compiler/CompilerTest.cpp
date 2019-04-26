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
    ASSERT_NO_THROW(C.compile(Program.get()));

    const auto ByteCode = C.byteCode();
    testInstructions(Test.ExpectedInstructions, ByteCode.Instructions);
    testConstants(Test.ExpectedConstants, ByteCode.Constants);
  }
}

TEST(CompilerTests, testIntegerArithmetic) {
  const std::vector<CompilerTestCase<int>> Tests = {
      {"1 + 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1; 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpPop, {}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 - 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpSub, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 * 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpMul, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"2 / 1",
       {2, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpDiv, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"-1",
       {1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpMinus, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testBooleanExpressions) {
  const std::vector<CompilerTestCase<int64_t>> Tests = {
      {"true",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"false",
       {},
       {code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 > 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpGreaterThan, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 < 2",
       {2, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpGreaterThan, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 == 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 != 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpNotEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"true == false",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"true != false",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpNotEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"!true",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpBang, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testConditionals) {
  const std::vector<CompilerTestCase<int64_t>> Tests = {
      {"if (true) { 10 }; 3333;",
       {10, 3333},
       {// 0000
        code::make(code::OpCode::OpTrue, {}),
        // 0001
        code::make(code::OpCode::OpJumpNotTruthy, {7}),
        // 0004
        code::make(code::OpCode::OpConstant, {0}),
        // 0007
        code::make(code::OpCode::OpPop, {}),
        // 0008
        code::make(code::OpCode::OpConstant, {1}),
        // 0011
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

} // namespace monkey::compiler::test
