#include "Parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::parser::test {

void testLetStatement(ast::Statement *S, const std::string &Name) {
  EXPECT_THAT(S, testing::NotNull());
  EXPECT_EQ(S->tokenLiteral(), "let");

  auto *LetS = dynamic_cast<ast::LetStatement *>(S);
  EXPECT_THAT(LetS, testing::NotNull());

  EXPECT_EQ(LetS->Name->Value, Name);
  EXPECT_EQ(LetS->Name->tokenLiteral(), Name);
}

void testIntegerLiteral(ast::Expression *E, int64_t Value) {
  auto *I = dynamic_cast<ast::IntegerLiteral *>(E);
  EXPECT_THAT(I, testing::NotNull());
  EXPECT_EQ(I->Value, Value);
  EXPECT_EQ(I->tokenLiteral(), std::to_string(Value));
}

void testIdentifier(ast::Expression *E, const std::string &Value) {
  auto *I = dynamic_cast<ast::Identifier *>(E);
  EXPECT_THAT(I, testing::NotNull());
  EXPECT_EQ(I->Value, Value);
  EXPECT_EQ(I->tokenLiteral(), Value);
}

void testBooleanLiteral(ast::Expression *E, bool Value) {
  auto *B = dynamic_cast<ast::Boolean *>(E);
  EXPECT_THAT(B, testing::NotNull());
  EXPECT_EQ(B->Value, Value);
  EXPECT_EQ(B->tokenLiteral(), Value ? "true" : "false");
}

void testLiteralExpression(ast::Expression *E, int64_t Expected) {
  testIntegerLiteral(E, Expected);
}

void testLiteralExpression(ast::Expression *E, const std::string &Expected) {
  testIdentifier(E, Expected);
}

void testLiteralExpression(ast::Expression *E, const char *Expected) {
  testIdentifier(E, std::string(Expected));
}

void testLiteralExpression(ast::Expression *E, bool Expected) {
  testBooleanLiteral(E, Expected);
}

template <typename T0, typename T1>
void testInfixExpression(ast::Expression *E, const T0 &Left,
                         const std::string &Operator, const T1 &Right) {
  auto *IE = dynamic_cast<ast::InfixExpression *>(E);
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

template <typename T>
void testLetStatementsImpl(
    const std::tuple<std::string, std::string, T> &Test) {
  lexer::Lexer L(std::get<0>(Test));
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);
  EXPECT_THAT(Program.get(), testing::NotNull());
  EXPECT_EQ(Program->Statements.size(), 1);

  testLetStatement(Program->Statements.front().get(), std::get<1>(Test));

  auto *LetS =
      dynamic_cast<ast::LetStatement *>(Program->Statements.front().get());
  EXPECT_THAT(LetS, testing::NotNull());
  testLiteralExpression(LetS->Value.get(), std::get<2>(Test));
}

TEST(ParserTests, testLetStatements) {
  testLetStatementsImpl<int64_t>({"let x = 5;", "x", 5});
  testLetStatementsImpl<bool>({"let y = true;", "y", true});
  testLetStatementsImpl<std::string>({"let foobar = y;", "foobar", "y"});
}

template <typename T>
void testReturnStatementsImpl(const std::pair<std::string, T> &Test) {
  const std::string Input(std::get<0>(Test));

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);
  EXPECT_THAT(Program.get(), testing::NotNull());
  EXPECT_EQ(Program->Statements.size(), 1);

  auto *Return =
      dynamic_cast<ast::ReturnStatement *>(Program->Statements.front().get());
  EXPECT_THAT(Return, testing::NotNull());
  EXPECT_EQ(Return->tokenLiteral(), "return");
  testLiteralExpression(Return->ReturnValue.get(), std::get<1>(Test));
}

TEST(ParserTests, testReturnStatements) {
  testReturnStatementsImpl<int64_t>({"return 5;", 5});
  testReturnStatementsImpl<bool>({"return false;", false});
  testReturnStatementsImpl<std::string>({"return foo;", "foo"});
}

TEST(ParserTests, testIdentifierExpression) {
  const std::string Input("foobar;");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *E = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<ast::Identifier *>(E->Expr.get());
  EXPECT_THAT(I, testing::NotNull());

  EXPECT_EQ(I->Value, "foobar");
  EXPECT_EQ(I->tokenLiteral(), "foobar");
}

TEST(ParserTests, testIntegerLiteralExpression) {
  const std::string Input("5;");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *E = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<ast::IntegerLiteral *>(E->Expr.get());
  EXPECT_THAT(I, testing::NotNull());

  EXPECT_EQ(I->Value, 5);
  EXPECT_EQ(I->tokenLiteral(), "5");
}

TEST(ParserTests, testParsingPrefixExpressions) {
  const std::vector<std::tuple<std::string, std::string, int64_t>> Tests = {
      {"!5", "!", 5}, {"-15", "-", 15}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    EXPECT_THAT(E, testing::NotNull());

    auto *PE = dynamic_cast<ast::PrefixExpression *>(E->Expr.get());
    EXPECT_THAT(PE, testing::NotNull());

    EXPECT_EQ(PE->Operator, std::get<1>(Test));

    testIntegerLiteral(PE->Right.get(), std::get<2>(Test));
  }

  const std::vector<std::tuple<std::string, std::string, bool>> BooleanTests = {
      {"!true", "!", true}, {"!false", "!", false}};

  for (const auto &Test : BooleanTests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    EXPECT_THAT(E, testing::NotNull());

    auto *PE = dynamic_cast<ast::PrefixExpression *>(E->Expr.get());
    EXPECT_THAT(PE, testing::NotNull());

    EXPECT_EQ(PE->Operator, std::get<1>(Test));

    testBooleanLiteral(PE->Right.get(), std::get<2>(Test));
  }
}

TEST(ParserTests, testParsingInfixExpressions) {
  const std::vector<std::tuple<std::string, int64_t, std::string, int64_t>>
      Tests = {{"5 + 5", 5, "+", 5},   {"5 - 5", 5, "-", 5},
               {"5 * 5", 5, "*", 5},   {"5 / 5", 5, "/", 5},
               {"5 > 5", 5, ">", 5},   {"5 < 5", 5, "<", 5},
               {"5 == 5", 5, "==", 5}, {"5 != 5", 5, "!=", 5}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    EXPECT_THAT(E, testing::NotNull());

    testInfixExpression(E->Expr.get(), std::get<1>(Test), std::get<2>(Test),
                        std::get<3>(Test));
  }

  const std::vector<std::tuple<std::string, bool, std::string, bool>>
      BooleanTests = {{"true == true", true, "==", true},
                      {"true != false", true, "!=", false},
                      {"false == false", false, "==", false}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    EXPECT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
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
      {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
      {"true", "true"},
      {"false", "false"},
      {"3 > 5 == false", "((3 > 5) == false)"},
      {"3 < 5 == true", "((3 < 5) == true)"},
      {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
      {"(5 + 5) * 2", "((5 + 5) * 2)"},
      {"2 / (5 + 5)", "(2 / (5 + 5))"},
      {"-(5 + 5)", "(-(5 + 5))"},
      {"!(true == true)", "(!(true == true))"},
      {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
      {"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
       "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
      {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    auto Actual = Program->string();
    EXPECT_EQ(Actual, std::get<1>(Test));
  }
}

TEST(ParserTests, testIfExpression) {
  const std::string Input("if (x < y) { x }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(ES, testing::NotNull());

  auto *IfE = dynamic_cast<ast::IfExpression *>(ES->Expr.get());
  EXPECT_THAT(IfE, testing::NotNull());

  testInfixExpression(IfE->Condition.get(), "x", "<", "y");

  EXPECT_EQ(IfE->Consequence->Statements.size(), 1);

  auto *Cons = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Consequence->Statements.front().get());
  EXPECT_THAT(Cons, testing::NotNull());

  testIdentifier(Cons->Expr.get(), "x");
  EXPECT_THAT(IfE->Alternative.get(), testing::IsNull());
}

TEST(ParserTests, testIfElseExpression) {
  const std::string Input("if (x < y) { x } else { y }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(ES, testing::NotNull());

  auto *IfE = dynamic_cast<ast::IfExpression *>(ES->Expr.get());
  EXPECT_THAT(IfE, testing::NotNull());

  testInfixExpression(IfE->Condition.get(), "x", "<", "y");

  EXPECT_EQ(IfE->Consequence->Statements.size(), 1);

  auto *Cons = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Consequence->Statements.front().get());
  EXPECT_THAT(Cons, testing::NotNull());

  testIdentifier(Cons->Expr.get(), "x");

  EXPECT_EQ(IfE->Alternative->Statements.size(), 1);
  auto *Alt = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Alternative->Statements.front().get());
  EXPECT_THAT(Alt, testing::NotNull());

  testIdentifier(Alt->Expr.get(), "y");
}

TEST(ParserTests, testFunctionLiteralParsing) {
  const std::string Input("fn(x, y) { x + y; }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(ES, testing::NotNull());

  auto *Function = dynamic_cast<ast::FunctionLiteral *>(ES->Expr.get());
  EXPECT_THAT(Function, testing::NotNull());

  EXPECT_EQ(Function->Parameters.size(), 2);

  testLiteralExpression(Function->Parameters[0].get(), "x");
  testLiteralExpression(Function->Parameters[1].get(), "y");

  EXPECT_EQ(Function->Body->Statements.size(), 1);

  auto *Body = dynamic_cast<ast::ExpressionStatement *>(
      Function->Body->Statements.front().get());
  EXPECT_THAT(Body, testing::NotNull());

  testInfixExpression(Body->Expr.get(), "x", "+", "y");
}

TEST(ParserTests, testFunctionParameterParsing) {
  const std::vector<std::pair<std::string, std::vector<std::string>>> Tests = {
      {"fn() {};", {}},
      {"fn(x) {};", {"x"}},
      {"fn(x, y, z) {};", {"x", "y", "z"}}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    auto *Statement = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    EXPECT_THAT(Statement, testing::NotNull());

    auto *Function =
        dynamic_cast<ast::FunctionLiteral *>(Statement->Expr.get());
    EXPECT_THAT(Function, testing::NotNull());

    EXPECT_EQ(Function->Parameters.size(), std::get<1>(Test).size());

    for (unsigned int Index = 0; Index < Function->Parameters.size(); ++Index) {
      testLiteralExpression(Function->Parameters.at(Index).get(),
                            std::get<1>(Test).at(Index));
    }
  }
}

TEST(ParserTests, testCallExpressionParsing) {
  const std::string Input("add(1, 2 * 3, 4 + 5)");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  EXPECT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  EXPECT_THAT(ES, testing::NotNull());

  auto *Call = dynamic_cast<ast::CallExpression *>(ES->Expr.get());
  EXPECT_THAT(Call, testing::NotNull());

  testIdentifier(Call->Function.get(), "add");

  EXPECT_EQ(Call->Arguments.size(), 3);

  testLiteralExpression(Call->Arguments[0].get(), (int64_t)1);
  testInfixExpression(Call->Arguments[1].get(), (int64_t)2, "*", (int64_t)3);
  testInfixExpression(Call->Arguments[2].get(), (int64_t)4, "+", (int64_t)5);
}

} // namespace monkey::parser::test
