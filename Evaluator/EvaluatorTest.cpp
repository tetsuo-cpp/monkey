#include <Evaluator/Evaluator.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::evaluator {

std::unique_ptr<object::Object> testEval(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  return eval(Program.get());
}

void testIntegerObject(const object::Object *Obj, int64_t Expected) {
  const auto *IntegerObj = dynamic_cast<const object::Integer *>(Obj);
  EXPECT_THAT(IntegerObj, testing::NotNull());
  EXPECT_EQ(IntegerObj->Value, Expected);
}

void testBooleanObject(const object::Object *Obj, bool Expected) {
  const auto *BooleanObj = dynamic_cast<const object::Boolean *>(Obj);
  EXPECT_THAT(BooleanObj, testing::NotNull());
  EXPECT_EQ(BooleanObj->Value, Expected);
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
      {"true", true},    {"false", false}, {"1 < 2", true},  {"1 > 2", false},
      {"1 < 1", false},  {"1 > 1", false}, {"1 == 1", true}, {"1 != 1", false},
      {"1 == 2", false}, {"1 != 2", true}};

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

} // namespace monkey::evaluator
