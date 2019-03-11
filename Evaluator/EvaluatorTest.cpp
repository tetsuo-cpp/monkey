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

TEST(EvaluatorTests, testEvalIntegerExpressions) {
  const std::vector<std::pair<std::string, int64_t>> Tests = {{"5", 5},
                                                              {"10", 10}};

  for (const auto &Test : Tests) {
    auto Evaluated = testEval(std::get<0>(Test));
    testIntegerObject(Evaluated.get(), std::get<1>(Test));
  }
}

} // namespace monkey::evaluator
