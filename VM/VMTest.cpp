#include "VM.h"

#include <AST/AST.h>
#include <Compiler/Compiler.h>
#include <Lexer/Lexer.h>
#include <Object/Object.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::vm::test {

std::unique_ptr<ast::Program> parse(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  return P.parseProgram();
}

void testIntegerObject(int64_t Expected, const object::Object *Obj) {
  const auto *Integer = dynamic_cast<const object::Integer *>(Obj);
  ASSERT_THAT(Integer, testing::NotNull());
  ASSERT_EQ(Integer->Value, Expected);
}

void testBooleanObject(bool Expected, const object::Object *Obj) {
  const auto *Boolean = dynamic_cast<const object::Boolean *>(Obj);
  ASSERT_THAT(Boolean, testing::NotNull());
  ASSERT_EQ(Boolean->Value, Expected);
}

template <typename T> struct VMTestCase {
  const std::string Input;
  const T Expected;
};

void testExpectedObject(int64_t Expected, const object::Object *Obj) {
  testIntegerObject(Expected, Obj);
}

void testExpectedObject(bool Expected, const object::Object *Obj) {
  testBooleanObject(Expected, Obj);
}

template <typename T> void runVMTests(const std::vector<VMTestCase<T>> &Tests) {
  for (const auto &Test : Tests) {
    auto Program = parse(Test.Input);

    compiler::Compiler C;
    ASSERT_NO_THROW(C.compile(Program.get()));

    VM VM(C.byteCode());
    ASSERT_NO_THROW(VM.run());

    const auto *StackElem = VM.lastPoppedStackElem();
    testExpectedObject(Test.Expected, StackElem);
  }
}

TEST(VMTests, testIntegerArithmetic) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"1", 1},
      {"2", 2},
      {"1 + 2", 3},
      {"1 - 2", -1},
      {"1 * 2", 2},
      {"4 / 2", 2},
      {"50 / 2 * 2 + 10 - 5", 55},
      {"5 * (2 + 10)", 60},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"5 * (2 + 10)", 60},
      {"-5", -5},
      {"-10", -10},
      {"-50 + 100 + -50", 0},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50}};

  runVMTests(Tests);
}

TEST(VMTests, testBooleanExpressions) {
  const std::vector<VMTestCase<bool>> Tests = {{"true", true},
                                               {"false", false},
                                               {"1 < 2", true},
                                               {"1 > 2", false},
                                               {"1 < 1", false},
                                               {"1 > 1", false},
                                               {"1 == 1", true},
                                               {"1 != 1", false},
                                               {"1 == 2", false},
                                               {"1 != 2", true},
                                               {"true == true", true},
                                               {"false == false", true},
                                               {"true == false", false},
                                               {"true != false", true},
                                               {"false != true", true},
                                               {"(1 < 2) == true", true},
                                               {"(1 < 2) == false", false},
                                               {"(1 > 2) == true", false},
                                               {"(1 > 2) == false", true},
                                               {"!true", false},
                                               {"!false", true},
                                               {"!5", false},
                                               {"!!true", true},
                                               {"!!false", false},
                                               {"!!5", true}};

  runVMTests(Tests);
}

} // namespace monkey::vm::test
