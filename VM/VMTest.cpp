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

void testStringObject(const std::string &Expected, const object::Object *Obj) {
  const auto *String = dynamic_cast<const object::String *>(Obj);
  ASSERT_THAT(String, testing::NotNull());
  ASSERT_EQ(String->Value, Expected);
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

void testExpectedObject(void *, const object::Object *Obj) {
  testVoidObject(Obj);
}

void testExpectedObject(const std::string &Expected,
                        const object::Object *Obj) {
  testStringObject(Expected, Obj);
}

template <typename T>
void testExpectedObject(const std::vector<T> &Expected,
                        const object::Object *Obj) {
  const auto *ArrayL = dynamic_cast<const object::Array *>(Obj);
  ASSERT_THAT(ArrayL, testing::NotNull());
  ASSERT_EQ(ArrayL->Elements.size(), Expected.size());
  for (unsigned int I = 0; I < Expected.size(); ++I)
    testExpectedObject(Expected.at(I), ArrayL->Elements.at(I).get());
}

template <typename T>
void testExpectedObject(
    const std::vector<std::pair<object::HashKey, T>> &Expected,
    const object::Object *Obj) {
  const auto *HashL = dynamic_cast<const object::Hash *>(Obj);
  ASSERT_THAT(HashL, testing::NotNull());
  ASSERT_EQ(HashL->Pairs.size(), Expected.size());
  for (const auto &Exp : Expected) {
    const auto HashIter = HashL->Pairs.find(Exp.first);
    ASSERT_NE(HashIter, HashL->Pairs.end());
    testExpectedObject(Exp.second, HashIter->second.get());
  }
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

TEST(VMTests, testStringExpressions) {
  const std::vector<VMTestCase<std::string>> Tests = {
      {"\"monkey\"", "monkey"},
      {"\"mon\" + \"key\"", "monkey"},
      {"\"mon\" + \"key\" + \"banana\"", "monkeybanana"}};

  runVMTests(Tests);
}

TEST(VMTests, testArrayLiterals) {
  const std::vector<VMTestCase<std::vector<int64_t>>> Tests = {
      {"[]", {}},
      {"[1, 2, 3]", {1, 2, 3}},
      {"[1 + 2, 3 * 4, 5 + 6]", {3, 12, 11}}};

  runVMTests(Tests);
}

TEST(VMTests, testHashLiterals) {
  const std::vector<
      VMTestCase<std::vector<std::pair<object::HashKey, int64_t>>>>
      Tests = {{"{}", {}},
               {"{1: 2, 2: 3}",
                {{object::HashKey(std::make_shared<object::Integer>(1)), 2},
                 {object::HashKey(std::make_shared<object::Integer>(2)), 3}}},
               {"{1 + 1: 2 * 2, 3 + 3: 4 * 4}",
                {{object::HashKey(std::make_shared<object::Integer>(2)), 4},
                 {object::HashKey(std::make_shared<object::Integer>(6)), 16}}}};

  runVMTests(Tests);
}

TEST(VMTests, testIndexExpressions) {
  const std::vector<VMTestCase<int64_t>> Tests = {{"[1, 2, 3][1]", 2},
                                                  {"[1, 2, 3][0 + 2]", 3},
                                                  {"[[1, 1, 1]][0][0]", 1},
                                                  {"{1: 1, 2: 2}[1]", 1},
                                                  {"{1: 1, 2: 2}[2]", 2}};

  runVMTests(Tests);

  const std::vector<VMTestCase<void *>> NullTests = {{"[][0]", nullptr},
                                                     {"[1, 2, 3][99]", nullptr},
                                                     {"[1][-1]", nullptr},
                                                     {"{1: 1}[0]", nullptr},
                                                     {"{}[0]", nullptr}};

  runVMTests(NullTests);
}

TEST(VMTests, testCallingFunctionsWithoutArguments) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let fivePlusTen = fn() { 5 + 10; };"
       "fivePlusTen();",
       15},
      {"let one = fn() { 1; };"
       "let two = fn() { 2; };"
       "one() + two()",
       3},
      {"let a = fn() { 1 };"
       "let b = fn() { a() + 1 };"
       "let c = fn() { b() + 1 };"
       "c();",
       3}};

  runVMTests(Tests);
}

TEST(VMTests, testFunctionsWithReturnStatement) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let earlyExit = fn() { return 99; 100; };"
       "earlyExit();",
       99},
      {"let earlyExit = fn() { return 99; return 100; };"
       "earlyExit();",
       99}};

  runVMTests(Tests);
}

TEST(VMTests, testFunctionsWithoutReturnValue) {
  const std::vector<VMTestCase<void *>> Tests = {
      {"let noReturn = fn() { };"
       "noReturn();",
       nullptr},
      {"let noReturn = fn() { };"
       "let noReturnTwo = fn() { noReturn(); };"
       "noReturn();"
       "noReturnTwo();",
       nullptr}};

  runVMTests(Tests);
}

TEST(VMTests, testFirstClassFunctions) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let returnsOne = fn() { 1; };"
       "let returnsOneReturner = fn() { returnsOne; };"
       "returnsOneReturner()();",
       1},
      {"let returnsOneReturner = fn() {"
       "let returnsOne = fn() { 1; };"
       "returnsOne;"
       "};"
       "returnsOneReturner()();",
       1}};

  runVMTests(Tests);
}

TEST(VMTests, testCallingFunctionsWithBindings) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let one = fn() { let one = 1; one };"
       "one();",
       1},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
       "oneAndTwo();",
       3},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
       "let threeAndFour = fn() { let three = 3; let four = 4; three + four; };"
       "oneAndTwo() + threeAndFour();",
       10},
      {"let firstFoobar = fn() { let foobar = 50; foobar; };"
       "let secondFoobar = fn() { let foobar = 100; foobar; };"
       "firstFoobar() + secondFoobar();",
       150},
      {"let globalSeed = 50;"
       "let minusOne = fn() {"
       "let num = 1;"
       "globalSeed - num;"
       "}"
       "let minusTwo = fn() {"
       "let num = 2;"
       "globalSeed - num;"
       "}"
       "minusOne() + minusTwo();",
       97}};

  runVMTests(Tests);
}

TEST(VMTests, testCallingFunctionWithArgumentsAndBindings) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"let identity= fn(a) { a; };"
       "identity(4);",
       4},
      {"let sum = fn(a, b) { a + b; };"
       "sum(1, 2);",
       3},
      {"let sum = fn(a, b) {"
       "let c = a + b;"
       "c;"
       "};"
       "sum(1, 2);",
       3},
      {"let sum = fn(a, b) {"
       "let c = a + b;"
       "c;"
       "};"
       "sum (1, 2) + sum(3, 4);",
       10},
      {"let sum = fn(a, b) {"
       "let c = a + b;"
       "c;"
       "};"
       "let outer = fn() {"
       "sum(1, 2) + sum(3, 4);"
       "};"
       "outer();",
       10},
      {"let globalNum = 10;"
       "let sum = fn(a, b) {"
       "let c = a + b;"
       "c + globalNum;"
       "};"
       "let outer = fn() {"
       "sum(1, 2) + sum(3, 4) + globalNum;"
       "};"
       "outer() + globalNum;",
       50}};

  runVMTests(Tests);
}

TEST(VMTests, testCallingFunctionWithWrongArguments) {
  const std::vector<VMTestCase<std::string>> Tests = {
      {"fn() { 1; }(1);", "wrong number of arguments: want=0, got=1"},
      {"fn(a) { a; }();", "wrong number of arguments: want=1, got=0"},
      {"fn(a, b) { a + b; }(1);", "wrong number of arguments: want=2, got=1"}};

  for (const auto &Test : Tests) {
    const auto Program = parse(Test.Input);

    compiler::SymbolTable ST;
    std::vector<std::shared_ptr<object::Object>> Constants;
    compiler::Compiler C(ST, Constants);
    ASSERT_NO_THROW(C.compile(Program.get()));

    std::array<std::shared_ptr<object::Object>, GlobalsSize> Globals;
    VM VM(C.byteCode(), Globals);
    std::string Error;
    try {
      VM.run();
    } catch (const std::runtime_error &RE) {
      Error = RE.what();
    }

    ASSERT_FALSE(Error.empty());
    ASSERT_EQ(Error, Test.Expected);
  }
}

} // namespace monkey::vm::test
