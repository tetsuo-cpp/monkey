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

void testVoidObject(const object::Object *Obj) {
  const auto *Null = dynamic_cast<const object::Null *>(Obj);
  ASSERT_THAT(Null, testing::NotNull());
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

void testExpectedObject(void *Expected, const object::Object *Obj) {
  static_cast<void>(Expected);
  testVoidObject(Obj);
}

template <typename T> void runVMTests(const std::vector<VMTestCase<T>> &Tests) {
  for (const auto &Test : Tests) {
    auto Program = parse(Test.Input);

    compiler::SymbolTable ST;
    std::vector<std::shared_ptr<object::Object>> Constants;
    compiler::Compiler C(ST, Constants);
    ASSERT_NO_THROW(C.compile(Program.get()));

    std::array<std::shared_ptr<object::Object>, GlobalsSize> Globals;
    VM VM(C.byteCode(), Globals);
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
                                               {"!!5", true},
                                               {"!(if (false) { 5; })", true}};

  runVMTests(Tests);
}

TEST(VMTests, testConditionals) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"if (true) { 10 }", 10},
      {"if (true) { 10 } else { 20 }", 10},
      {"if (false) { 10 } else { 20 }", 20},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if ((if (false) { 10 })) { 10 } else { 20 }", 20}};

  runVMTests(Tests);

  const std::vector<VMTestCase<void *>> NullTests = {
      {"if (1 > 2) { 10 }", nullptr}, {"if (false) { 10 }", nullptr}};

  runVMTests(NullTests);
}

TEST(VMTests, testGlobalLetStatements) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let one = 1; one", 1},
      {"let one = 1; let two = 2; one + two", 3},
      {"let one = 1; let two = one + one; one + two", 3}};

  runVMTests(Tests);
}

} // namespace monkey::vm::test
