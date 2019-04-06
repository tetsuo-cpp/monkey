#include <Evaluator/Evaluator.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::evaluator {

std::shared_ptr<object::Object> testEval(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  environment::Environment Env;
  return eval(Program.get(), Env);
}

void testIntegerObject(const object::Object *Obj, int64_t Expected) {
  const auto *IntegerObj = dynamic_cast<const object::Integer *>(Obj);
  ASSERT_THAT(IntegerObj, testing::NotNull());
  ASSERT_EQ(IntegerObj->Value, Expected);
}

void testBooleanObject(const object::Object *Obj, bool Expected) {
  const auto *BooleanObj = dynamic_cast<const object::Boolean *>(Obj);
  ASSERT_THAT(BooleanObj, testing::NotNull());
  ASSERT_EQ(BooleanObj->Value, Expected);
}

void testNullObject(const object::Object *Obj) {
  const auto *NullObj = dynamic_cast<const object::Null *>(Obj);
  ASSERT_THAT(NullObj, testing::NotNull());
}

TEST(EvaluatorTests, testEvalIntegerExpressions) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"5", 5},
      {"10", 10},
      {"-5", -5},
      {"-10", -10},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"-50 + 100 + -50", 0},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"20 + 2 * -10", 0},
      {"50 / 2 * 2 + 10", 60},
      {"2 * (5 + 10)", 30},
      {"3 * 3 * 3 + 10", 37},
      {"3 * (3 * 3) + 10", 37},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testIntegerObject(Evaluated.get(), std::get<1>(Test));
  }
}

TEST(EvaluatorTests, testEvalBooleanExpressions) {
  const std::vector<std::pair<std::string, bool>> Tests = {
      {"true", true},
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
      {"(1 < 2) = true", true},
      {"(1 < 2) == false", false},
      {"(1 > 2) == true", false},
      {"(1 > 2) == false", true}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testBooleanObject(Evaluated.get(), std::get<1>(Test));
  }
}

TEST(EvaluatorTests, testBangOperator) {
  const std::vector<std::pair<std::string, bool>> Tests = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testBooleanObject(Evaluated.get(), std::get<1>(Test));
  }
}

void testIfElseExpressionInteger(const std::string &Input, int64_t Expected) {
  auto Evaluated = testEval(Input);
  testIntegerObject(Evaluated.get(), Expected);
}

void testIfElseExpressionNull(const std::string &Input) {
  auto Evaluated = testEval(Input);
  testNullObject(Evaluated.get());
}

TEST(EvaluatorTests, testIfElseExpressions) {
  testIfElseExpressionInteger("if (true) { 10 }", 10);
  testIfElseExpressionNull("if (false) { 10 }");
  testIfElseExpressionInteger("if (1) { 10 }", 10);
  testIfElseExpressionInteger("if (1 < 2) { 10 }", 10);
  testIfElseExpressionNull("if (1 > 2) { 10 }");
  testIfElseExpressionInteger("if (1 > 2) { 10 } else { 20 }", 20);
  testIfElseExpressionInteger("if (1 < 2) { 10 } else { 20 }", 10);
}

TEST(EvaluatorTests, testReturnStatements) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"return 10;", 10},
      {"return 10; 9", 10},
      {"return 2 * 5; 9", 10},
      {"9; return 2 * 5; 9", 10}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testIntegerObject(Evaluated.get(), std::get<1>(Test));
  }
}

TEST(EvaluatorTests, testErrorHandling) {
  const std::vector<std::pair<std::string, std::string>> Tests = {
      {"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
      {"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
      {"-true", "unknown operator: -BOOLEAN"},
      {"true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
      {"5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"},
      {"if (10 > 1) { true + false; }", "unknown operator: BOOLEAN + BOOLEAN"},
      {"if (10 > 1) {"
       "if (10 > 1) {"
       "return true + false;"
       "}"
       "return 1;"
       "}",
       "unknown operator: BOOLEAN + BOOLEAN"},
      {"foobar", "identifier not found: foobar"},
      {"\"Hello\" - \"World\"", "unknown operator: STRING - STRING"},
      {"len(1)", "argument to \"len\" not supported, got INTEGER"},
      {"len(\"one\", \"two\")", "wrong number of arguments. got=2, want=1"},
      {"{\"name\": \"Monkey\"}[fn(x) { x }];",
       "unusable as hash key: FUNCTION"}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    const auto *Error = dynamic_cast<object::Error *>(Evaluated.get());
    ASSERT_THAT(Error, testing::NotNull());
    ASSERT_EQ(Error->Message, std::get<1>(Test));
  }
}

TEST(EvaluatorTests, testLetStatements) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15}};

  for (const auto &Test : Tests)
    testIntegerObject(testEval(std::get<0>(Test)).get(), std::get<1>(Test));
}

TEST(EvaluatorTests, testFunctionObject) {
  const std::string Input("fn(x) { x + 2; };");
  auto Evaluated = testEval(Input);
  const auto *Function = dynamic_cast<object::Function *>(Evaluated.get());
  ASSERT_THAT(Function, testing::NotNull());
  ASSERT_EQ(Function->Parameters.size(), 1);
  ASSERT_EQ(Function->Parameters.front()->string(), "x");

  const std::string ExpectedBody("(x + 2)");
  ASSERT_EQ(Function->Body->string(), ExpectedBody);
}

TEST(EvaluatorTests, testFunctionApplication) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { x * 2; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
      {"fn(x) { x; }(5)", 5}};

  for (const auto &Test : Tests)
    testIntegerObject(testEval(std::get<0>(Test)).get(), std::get<1>(Test));
}

TEST(EvaluatorTests, testClosures) {
  const std::string Input("let newAdder = fn(x) {"
                          "fn(y) { x + y };"
                          "};"
                          "let addTwo = newAdder(2);"
                          "addTwo(2);");

  testIntegerObject(testEval(Input).get(), 4);
}

TEST(EvaluatorTests, testStringLiteral) {
  const std::string Input("\"Hello World\"");

  auto Evaluated = testEval(Input);
  const auto *S = dynamic_cast<const object::String *>(Evaluated.get());
  ASSERT_THAT(S, testing::NotNull());
  ASSERT_EQ(S->Value, "Hello World");
}

TEST(EvaluatorTests, testStringConcatenation) {
  const std::string Input("\"Hello\" + \" \" + \"World\"");

  auto Evaluated = testEval(Input);
  const auto *S = dynamic_cast<const object::String *>(Evaluated.get());
  ASSERT_THAT(S, testing::NotNull());
  ASSERT_EQ(S->Value, "Hello World");
}

TEST(EvaluatorTests, testBuiltinFunctions) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"len(\"\")", 0}, {"len(\"four\")", 4}, {"len(\"hello world\")", 11}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    const auto *S = dynamic_cast<const object::Integer *>(Evaluated.get());
    ASSERT_THAT(S, testing::NotNull());
    ASSERT_EQ(S->Value, std::get<1>(Test));
  }
}

TEST(EvaluatorTests, testArrayLiterals) {
  const std::string Input("[1, 2 * 2, 3 + 3]");

  auto Evaluated = testEval(Input);
  const auto *AL = dynamic_cast<const object::Array *>(Evaluated.get());
  ASSERT_THAT(AL, testing::NotNull());
  ASSERT_EQ(AL->Elements.size(), 3);
  testIntegerObject(AL->Elements.at(0).get(), 1);
  testIntegerObject(AL->Elements.at(1).get(), 4);
  testIntegerObject(AL->Elements.at(2).get(), 6);
}

TEST(EvaluatorTests, testArrayIndexExpressions) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"[1, 2, 3][0]", 1},
      {"[1, 2, 3][1]", 2},
      {"[1, 2, 3][2]", 3},
      {"let i = 0; [1][i];", 1},
      {"[1, 2, 3][1 + 1]", 3},
      {"let myArray = [1, 2, 3]; myArray[2];", 3},
      {"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6},
      {"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testIntegerObject(Evaluated.get(), std::get<1>(Test));
  }

  const std::vector<std::string> NullTests = {"[1, 2, 3][3]", "[1, 2, 3][-1]"};

  for (const auto &NullTest : NullTests) {
    auto Evaluated = testEval(NullTest);
    testNullObject(Evaluated.get());
  }
}

TEST(EvaluatorTests, testHashLiterals) {
  const std::string Input("let two = \"two\";"
                          "{"
                          "\"one\": 10 - 9,"
                          "\"two\": 1 + 1,"
                          "\"thr\" + \"ee\": 6 / 2,"
                          "4: 4,"
                          "true: 5,"
                          "false: 6"
                          "}");

  auto Evaluated = testEval(Input);
  const auto *Hash = dynamic_cast<const object::Hash *>(Evaluated.get());
  ASSERT_THAT(Hash, testing::NotNull());

  const std::vector<std::pair<object::HashKey, int64_t>> Expected = {
      {object::HashKey(std::make_shared<object::String>("one")), 1},
      {object::HashKey(std::make_shared<object::String>("two")), 2},
      {object::HashKey(std::make_shared<object::String>("three")), 3},
      {object::HashKey(std::make_shared<object::Integer>(4)), 4},
      {object::HashKey(std::make_shared<object::Boolean>(true)), 5},
      {object::HashKey(std::make_shared<object::Boolean>(false)), 6}};

  ASSERT_EQ(Hash->Pairs.size(), Expected.size());

  for (const auto &E : Expected) {
    const auto Iter = Hash->Pairs.find(E.first);
    ASSERT_NE(Iter, Hash->Pairs.end());
    testIntegerObject(Iter->second.get(), E.second);
  }
}

TEST(EvaluatorTests, testHashIndexExpressions) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {
      {"{\"foo\": 5}[\"foo\"]", 5},
      {"let key = \"foo\"; {\"foo\": 5}[key]", 5},
      {"{5: 5}[5]", 5},
      {"{true: 5}[true]", 5},
      {"{false: 5}[false]", 5}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testIntegerObject(Evaluated.get(), std::get<1>(Test));
  }

  const std::vector<std::string> NullTests = {"{\"foo\": 5}[\"bar\"]",
                                              "{}[\"foo\"]"};

  for (const auto &Test : NullTests) {
    auto Evaluated = testEval(Test);
    testNullObject(Evaluated.get());
  }
}

} // namespace monkey::evaluator
