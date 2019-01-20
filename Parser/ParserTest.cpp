#include "Parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::test {

void testLetStatement(Statement *S, const std::string &Name) {
  EXPECT_THAT(S, testing::NotNull());
  EXPECT_EQ(S->tokenLiteral(), "let");

  auto *LetS = dynamic_cast<LetStatement *>(S);
  EXPECT_THAT(LetS, testing::NotNull());

  EXPECT_EQ(LetS->Name->Value, Name);
  EXPECT_EQ(LetS->Name->tokenLiteral(), Name);
}

void checkParserErrors(Parser &P) {
  const auto &Errors = P.errors();
  EXPECT_TRUE(Errors.empty());
  for (const auto &E : Errors) {
    std::cout << "parser error: " << E << "\n";
  }
}

TEST(ParserTests, testLetStatements) {
  const std::string Input("let x = 5;"
                          "let y = 10;"
                          "let foobar = 838383;");

  Lexer L(Input);
  Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);
  EXPECT_THAT(Program.get(), testing::NotNull());
  EXPECT_EQ(Program->Statements.size(), 3);

  const std::vector<std::string> Tests = {"x", "y", "foobar"};

  for (unsigned int i = 0; i < Tests.size(); ++i) {
    const auto &T = Tests.at(i);
    auto &Statement = Program->Statements.at(i);
    testLetStatement(Statement.get(), T);
  }
}

TEST(ParserTests, testReturnStatements) {
  const std::string Input("return 5;"
                          "return 10;"
                          "return 993322;");

  Lexer L(Input);
  Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);
  EXPECT_THAT(Program.get(), testing::NotNull());
  EXPECT_EQ(Program->Statements.size(), 3);

  for (const auto &S : Program->Statements) {
    auto *ReturnS = dynamic_cast<ReturnStatement *>(S.get());
    EXPECT_THAT(ReturnS, testing::NotNull());
    EXPECT_EQ(ReturnS->tokenLiteral(), "return");
  }
}

} // namespace monkey::test
