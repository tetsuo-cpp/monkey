#include "VM.h"

#include <AST/AST.h>
#include <Compiler/Compiler.h>
#include <Lexer/Lexer.h>
#include <Object/Object.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <variant>

namespace monkey::vm::test {

std::unique_ptr<ast::Program> parse(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  return P.parseProgram();
}

void testIntegerObject(int Expected, const object::Object *Obj) {
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

void testArrayObject(const std::vector<int> &Expected,
                     const object::Object *Obj) {
  const auto *ArrayL = dynamic_cast<const object::Array *>(Obj);
  ASSERT_THAT(ArrayL, testing::NotNull());
  ASSERT_EQ(ArrayL->Elements.size(), Expected.size());
  for (unsigned int I = 0; I < Expected.size(); ++I)
    testIntegerObject(Expected.at(I), ArrayL->Elements.at(I).get());
}

void testHashObject(
    const std::vector<std::pair<object::HashKey, int>> &Expected,
    const object::Object *Obj) {
  const auto *HashL = dynamic_cast<const object::Hash *>(Obj);
  ASSERT_THAT(HashL, testing::NotNull());
  ASSERT_EQ(HashL->Pairs.size(), Expected.size());
  for (const auto &Exp : Expected) {
    const auto HashIter = HashL->Pairs.find(Exp.first);
    ASSERT_NE(HashIter, HashL->Pairs.end());
    testIntegerObject(Exp.second, HashIter->second.get());
  }
}

void testErrorObj(const std::shared_ptr<object::Error> &Expected,
                  const object::Object *Obj) {
  const auto *ErrorObj = dynamic_cast<const object::Error *>(Obj);
  ASSERT_THAT(ErrorObj, testing::NotNull());
  ASSERT_EQ(ErrorObj->Message, Expected->Message);
}

using ExpectedType =
    std::variant<int, bool, std::string, std::vector<int>, void *,
                 std::vector<std::pair<object::HashKey, int>>,
                 std::shared_ptr<object::Error>>;

struct VMTestCase {
  const std::string Input;
  const ExpectedType Expected;
};

template <typename... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts> Overloaded(Ts...)->Overloaded<Ts...>;

void testExpectedObject(const ExpectedType &Expected,
                        const object::Object *Obj) {
  std::visit(
      Overloaded{
          [Obj](const int Arg) { testIntegerObject(Arg, Obj); },
          [Obj](const bool Arg) { testBooleanObject(Arg, Obj); },
          [Obj](const std::string &Arg) { testStringObject(Arg, Obj); },
          [Obj](const std::vector<int> &Arg) { testArrayObject(Arg, Obj); },
          [Obj](const void *) { testVoidObject(Obj); },
          [Obj](const std::vector<std::pair<object::HashKey, int>> &Arg) {
            testHashObject(Arg, Obj);
          },
          [Obj](const std::shared_ptr<object::Error> &Arg) {
            testErrorObj(Arg, Obj);
          }},
      Expected);
}

class TestVM : public VM {
public:
  TestVM(compiler::ByteCode &&BC,
         std::array<std::shared_ptr<object::Object>, GLOBALS_SIZE> &Globals)
      : VM(std::move(BC), Globals) {}
  virtual ~TestVM() = default;

  void push(const std::shared_ptr<object::Object> &Obj) { VM::push(Obj); }
  const std::shared_ptr<object::Object> &pop() override { return VM::pop(); }
};

void runVMTests(const std::vector<VMTestCase> &Tests) {
  for (const auto &Test : Tests) {
    auto Program = parse(Test.Input);

    compiler::SymbolTable ST;
    std::vector<std::shared_ptr<object::Object>> Constants;
    compiler::Compiler C(ST, Constants);
    ASSERT_NO_THROW(C.compile(Program.get()));

    std::array<std::shared_ptr<object::Object>, GLOBALS_SIZE> Globals;
    TestVM VM(C.byteCode(), Globals);
    ASSERT_NO_THROW(VM.run());

    const auto *StackElem = VM.lastPoppedStackElem();
    testExpectedObject(Test.Expected, StackElem);
  }
}

TEST(VMTests, testIntegerArithmetic) {
  const std::vector<VMTestCase> Tests = {
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
  const std::vector<VMTestCase> Tests = {{"true", true},
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
  const std::vector<VMTestCase> Tests = {
      {"if (true) { 10 }", 10},
      {"if (true) { 10 } else { 20 }", 10},
      {"if (false) { 10 } else { 20 }", 20},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if ((if (false) { 10 })) { 10 } else { 20 }", 20},
      {"if (1 > 2) { 10 }", nullptr},
      {"if (false) { 10 }", nullptr},
  };

  runVMTests(Tests);
}

TEST(VMTests, testGlobalLetStatements) {
  const std::vector<VMTestCase> Tests = {
      {"let one = 1; one", 1},
      {"let one = 1; let two = 2; one + two", 3},
      {"let one = 1; let two = one + one; one + two", 3}};

  runVMTests(Tests);
}

TEST(VMTests, testStringExpressions) {
  const std::vector<VMTestCase> Tests = {
      {"\"monkey\"", std::string("monkey")},
      {"\"mon\" + \"key\"", std::string("monkey")},
      {"\"mon\" + \"key\" + \"banana\"", std::string("monkeybanana")}};

  runVMTests(Tests);
}

TEST(VMTests, testArrayLiterals) {
  const std::vector<VMTestCase> Tests = {
      {"[]", std::vector<int>{}},
      {"[1, 2, 3]", std::vector<int>{1, 2, 3}},
      {"[1 + 2, 3 * 4, 5 + 6]", std::vector<int>{3, 12, 11}}};

  runVMTests(Tests);
}

TEST(VMTests, testHashLiterals) {
  const std::vector<VMTestCase> Tests = {
      {"{}", std::vector<std::pair<object::HashKey, int>>{}},
      {"{1: 2, 2: 3}",
       std::vector<std::pair<object::HashKey, int>>{
           {object::HashKey(std::make_shared<object::Integer>(1)), 2},
           {object::HashKey(std::make_shared<object::Integer>(2)), 3}}},
      {"{1 + 1: 2 * 2, 3 + 3: 4 * 4}",
       std::vector<std::pair<object::HashKey, int>>{
           {object::HashKey(std::make_shared<object::Integer>(2)), 4},
           {object::HashKey(std::make_shared<object::Integer>(6)), 16}}}};

  runVMTests(Tests);
}

TEST(VMTests, testIndexExpressions) {
  const std::vector<VMTestCase> Tests = {
      {"[1, 2, 3][1]", 2},        {"[1, 2, 3][0 + 2]", 3},
      {"[[1, 1, 1]][0][0]", 1},   {"{1: 1, 2: 2}[1]", 1},
      {"{1: 1, 2: 2}[2]", 2},     {"[][0]", nullptr},
      {"[1, 2, 3][99]", nullptr}, {"[1][-1]", nullptr},
      {"{1: 1}[0]", nullptr},     {"{}[0]", nullptr}};

  runVMTests(Tests);
}

TEST(VMTests, testCallingFunctionsWithoutArguments) {
  const std::vector<VMTestCase> Tests = {{"let fivePlusTen = fn() { 5 + 10; };"
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
  const std::vector<VMTestCase> Tests = {
      {"let earlyExit = fn() { return 99; 100; };"
       "earlyExit();",
       99},
      {"let earlyExit = fn() { return 99; return 100; };"
       "earlyExit();",
       99}};

  runVMTests(Tests);
}

TEST(VMTests, testFunctionsWithoutReturnValue) {
  const std::vector<VMTestCase> Tests = {
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
  const std::vector<VMTestCase> Tests = {
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
  const std::vector<VMTestCase> Tests = {
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
  const std::vector<VMTestCase> Tests = {{"let identity= fn(a) { a; };"
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
  const std::vector<std::pair<std::string, std::string>> Tests = {
      {"fn() { 1; }(1);", "wrong number of arguments: want=0, got=1"},
      {"fn(a) { a; }();", "wrong number of arguments: want=1, got=0"},
      {"fn(a, b) { a + b; }(1);", "wrong number of arguments: want=2, got=1"}};

  for (const auto &Test : Tests) {
    const auto Program = parse(Test.first);

    compiler::SymbolTable ST;
    std::vector<std::shared_ptr<object::Object>> Constants;
    compiler::Compiler C(ST, Constants);
    ASSERT_NO_THROW(C.compile(Program.get()));

    std::array<std::shared_ptr<object::Object>, GLOBALS_SIZE> Globals;
    VM VM(C.byteCode(), Globals);
    std::string Error;
    try {
      VM.run();
    } catch (const std::runtime_error &RE) {
      Error = RE.what();
    }

    ASSERT_FALSE(Error.empty());
    ASSERT_EQ(Error, Test.second);
  }
}

TEST(VMTests, testBuiltInFunctions) {
  const std::vector<VMTestCase> Tests = {
      {"len(\"\")", 0},
      {"len(\"four\")", 4},
      {"len(\"hello world\")", 11},
      {"len([1, 2, 3])", 3},
      {"len([])", 0},
      {"first([1, 2, 3])", 1},
      {"last([1, 2, 3])", 3},
      {"rest([1, 2, 3])", std::vector<int>{2, 3}},
      {"push([], 1)", std::vector<int>{1}},
      {"puts(\"hello\", \"world\")", nullptr},
      {"first([])", nullptr},
      {"last([])", nullptr},
      {"rest([])", nullptr},
      {"len(1)", std::make_shared<object::Error>(
                     "argument to \"len\" not supported, got INTEGER")},
      {"len(\"one\", \"two\")",
       std::make_shared<object::Error>(
           "wrong number of arguments. got=2, want=1")},
      {"first(1)", std::make_shared<object::Error>(
                       "argument to \"first\" must be ARRAY, got INTEGER")},
      {"last(1)", std::make_shared<object::Error>(
                      "argument to \"last\" must be ARRAY, got INTEGER")},
      {"push(1, 1)", std::make_shared<object::Error>(
                         "argument to \"push\" must be ARRAY, got INTEGER")}};

  runVMTests(Tests);
}

TEST(VMTests, testClosures) {
  const std::vector<VMTestCase> Tests = {
      {"let newClosure = fn(a) {"
       "fn() { a; };"
       "};"
       "let closure = newClosure(99);"
       "closure();",
       99},
      {"let newAdder = fn(a, b) {"
       "let c = a + b;"
       "fn(d) { c + d };"
       "};"
       "let adder = newAdder(1, 2);"
       "adder(8);",
       11},
      {"let newAdderOuter = fn(a, b) {"
       "let c = a + b;"
       "fn(d) {"
       "let e = d + c;"
       "fn(f) { e + f; };"
       "};"
       "};"
       "let newAdderInner = newAdderOuter(1, 2);"
       "let adder = newAdderInner(3);"
       "adder(8);",
       14},
      {"let a = 1;"
       "let newAdderOuter = fn(b) {"
       "fn(c) {"
       "fn(d) { a + b + c + d };"
       "};"
       "};"
       "let newAdderInner = newAdderOuter(2);"
       "let adder = newAdderInner(3);"
       "adder(8);",
       14},
      {"let newClosure = fn(a, b) {"
       "let one = fn() { a; };"
       "let two = fn() { b; };"
       "fn() { one() + two(); };"
       "};"
       "let closure = newClosure(9, 90);"
       "closure();",
       99}};

  runVMTests(Tests);
}

TEST(VMTests, testRecursiveFibonacci) {
  const std::vector<VMTestCase> Tests = {{"let fibonacci = fn(x) {"
                                          "if (x == 0) {"
                                          "return 0;"
                                          "} else {"
                                          "if (x == 1) {"
                                          "return 1;"
                                          "} else {"
                                          "fibonacci(x - 1) + fibonacci(x - 2);"
                                          "}"
                                          "}"
                                          "};"
                                          "fibonacci(15);",
                                          610}};

  runVMTests(Tests);
}

} // namespace monkey::vm::test
