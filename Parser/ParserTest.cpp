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

void testIntegerLiteral(Expression *E, int64_t Value) {
  auto *I = dynamic_cast<IntegerLiteral *>(E);
  EXPECT_THAT(I, testing::NotNull());
  EXPECT_EQ(I->Value, Value);
  EXPECT_EQ(I->tokenLiteral(), std::to_string(Value));
}

void testIdentifier(Expression *E, const std::string &Value) {
  auto *I = dynamic_cast<Identifier *>(E);
  EXPECT_THAT(I, testing::NotNull());
  EXPECT_EQ(I->Value, Value);
  EXPECT_EQ(I->tokenLiteral(), Value);
}

void testLiteralExpression(Expression *E, int Expected) {
  testIntegerLiteral(E, Expected);
}

void testLiteralExpression(Expression *E, const std::string &Expected) {
  testIdentifier(E, Expected);
}

template <typename T0, typename T1>
void testInfixExpression(Expression *E, const T0 &Left,
                         const std::string &Operator, const T1 &Right) {
  auto *IE = dynamic_cast<InfixExpression *>(E);
  EXPECT_THAT(IE, testing::NotNull());

  testLiteralExpression(IE->Left.get(), Left);
  EXPECT_EQ(IE->Operator, Operator);
  testLiteralExpression(IE->Right.get(), Right);
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

TEST(ParserTests, testIdentifierExpression) {
  const std::string Input("foobar;");

  Lexer L(Input);
  Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *E =
      dynamic_cast<ExpressionStatement *>(Program->Statements.front().get());
  EXPECT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<Identifier *>(E->Expr.get());
  EXPECT_THAT(I, testing::NotNull());

  EXPECT_EQ(I->Value, "foobar");
  EXPECT_EQ(I->tokenLiteral(), "foobar");
}

TEST(ParserTests, testIntegerLiteralExpression) {
  const std::string Input("5;");

  Lexer L(Input);
  Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *E =
      dynamic_cast<ExpressionStatement *>(Program->Statements.front().get());
  EXPECT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<IntegerLiteral *>(E->Expr.get());
  EXPECT_THAT(I, testing::NotNull());

  EXPECT_EQ(I->Value, 5);
  EXPECT_EQ(I->tokenLiteral(), "5");
}

TEST(ParserTests, testingParsingPrefixExpressions) {
  const std::vector<std::tuple<std::string, std::string, int64_t>> Tests = {
      {"!5", "!", 5}, {"-15", "-", 15}};

  for (const auto &Test : Tests) {
    Lexer L(std::get<0>(Test));
    Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E =
        dynamic_cast<ExpressionStatement *>(Program->Statements.front().get());
    EXPECT_THAT(E, testing::NotNull());

    auto *PE = dynamic_cast<PrefixExpression *>(E->Expr.get());
    EXPECT_THAT(PE, testing::NotNull());

    EXPECT_EQ(PE->Operator, std::get<1>(Test));

    testIntegerLiteral(PE->Right.get(), std::get<2>(Test));
  }
}

TEST(ParserTests, testingParsingInfixExpressions) {
  const std::vector<std::tuple<std::string, int64_t, std::string, int64_t>>
      Tests = {{"5 + 5", 5, "+", 5},   {"5 - 5", 5, "-", 5},
               {"5 * 5", 5, "*", 5},   {"5 / 5", 5, "/", 5},
               {"5 > 5", 5, ">", 5},   {"5 < 5", 5, "<", 5},
               {"5 == 5", 5, "==", 5}, {"5 != 5", 5, "!=", 5}};

  for (const auto &Test : Tests) {
    Lexer L(std::get<0>(Test));
    Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E =
        dynamic_cast<ExpressionStatement *>(Program->Statements.front().get());
    EXPECT_THAT(E, testing::NotNull());

    testInfixExpression(E->Expr.get(), std::get<1>(Test), std::get<2>(Test),
                        std::get<3>(Test));
  }
}

TEST(ParserTests, testOperatorPrecedenceParsing) {
  const std::vector<std::pair<std::string, std::string>> Tests = {
      {"-a * b", "((-a) * b)"},
      {"!-a", "(!(-a))"},
      {"a + b + c", "((a + b) + c)"},
      {"a + b - c", "((a + b) - c)"},
      {"a * b * c", "((a * b) * c)"},
      {"a * b / c", "((a * b) / c)"},
      {"a + b / c", "(a + (b / c))"},
      {"a + b * c + d / e -f", "(((a + (b * c)) + (d / e)) - f)"},
      {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
      {"5 > 4 == 3 > 4", "((5 > 4) == (3 > 4))"},
      {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
      {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"}};

  for (const auto &Test : Tests) {
    Lexer L(std::get<0>(Test));
    Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    auto Actual = Program->string();
    EXPECT_EQ(Actual, std::get<1>(Test));
  }
}

} // namespace monkey::test
