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

template <typename T> struct VMTestCase {
  const std::string Input;
  const T Expected;
};

void testExpectedObject(int64_t Expected, const object::Object *Obj) {
  testIntegerObject(Expected, Obj);
}

template <typename T> void runVMTests(const std::vector<VMTestCase<T>> &Tests) {
  for (const auto &Test : Tests) {
    auto Program = parse(Test.Input);

    compiler::Compiler C;
    ASSERT_NO_THROW(C.compile(Program.get()));

    VM VM(C.byteCode());
    ASSERT_NO_THROW(VM.run());

    const auto *StackElem = VM.stackTop();
    testExpectedObject(Test.Expected, StackElem);
  }
}

TEST(VMTests, testIntegerArithmetic) {
  const std::vector<VMTestCase<int64_t>> Tests = {
      {"1", 1}, {"2", 2}, {"1 + 2", 3}};

  runVMTests(Tests);
}

} // namespace monkey::vm::test
