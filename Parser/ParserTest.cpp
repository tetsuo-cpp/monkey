#include "Parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::parser::test {

void testLetStatement(ast::Statement *S, const std::string &Name) {
  ASSERT_THAT(S, testing::NotNull());
  ASSERT_EQ(S->tokenLiteral(), "let");

  auto *LetS = dynamic_cast<ast::LetStatement *>(S);
  ASSERT_THAT(LetS, testing::NotNull());

  ASSERT_EQ(LetS->Name->Value, Name);
  ASSERT_EQ(LetS->Name->tokenLiteral(), Name);
}

void testIntegerLiteral(ast::Expression *E, int64_t Value) {
  auto *I = dynamic_cast<ast::IntegerLiteral *>(E);
  ASSERT_THAT(I, testing::NotNull());
  ASSERT_EQ(I->Value, Value);
  ASSERT_EQ(I->tokenLiteral(), std::to_string(Value));
}

void testIdentifier(ast::Expression *E, const std::string &Value) {
  auto *I = dynamic_cast<ast::Identifier *>(E);
  ASSERT_THAT(I, testing::NotNull());
  ASSERT_EQ(I->Value, Value);
  ASSERT_EQ(I->tokenLiteral(), Value);
}

void testBooleanLiteral(ast::Expression *E, bool Value) {
  auto *B = dynamic_cast<ast::Boolean *>(E);
  ASSERT_THAT(B, testing::NotNull());
  ASSERT_EQ(B->Value, Value);
  ASSERT_EQ(B->tokenLiteral(), Value ? "true" : "false");
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
  ASSERT_THAT(IE, testing::NotNull());

  testLiteralExpression(IE->Left.get(), Left);
  ASSERT_EQ(IE->Operator, Operator);
  testLiteralExpression(IE->Right.get(), Right);
}

void checkParserErrors(Parser &P) {
  const auto &Errors = P.errors();
  ASSERT_TRUE(Errors.empty());
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
  ASSERT_THAT(Program.get(), testing::NotNull());
  ASSERT_EQ(Program->Statements.size(), 1);

  testLetStatement(Program->Statements.front().get(), std::get<1>(Test));

  auto *LetS =
      dynamic_cast<ast::LetStatement *>(Program->Statements.front().get());
  ASSERT_THAT(LetS, testing::NotNull());
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
  ASSERT_THAT(Program.get(), testing::NotNull());
  ASSERT_EQ(Program->Statements.size(), 1);

  auto *Return =
      dynamic_cast<ast::ReturnStatement *>(Program->Statements.front().get());
  ASSERT_THAT(Return, testing::NotNull());
  ASSERT_EQ(Return->tokenLiteral(), "return");
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

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *E = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<ast::Identifier *>(E->Expr.get());
  ASSERT_THAT(I, testing::NotNull());

  ASSERT_EQ(I->Value, "foobar");
  ASSERT_EQ(I->tokenLiteral(), "foobar");
}

TEST(ParserTests, testIntegerLiteralExpression) {
  const std::string Input("5;");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *E = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(E, testing::NotNull());

  auto *I = dynamic_cast<ast::IntegerLiteral *>(E->Expr.get());
  ASSERT_THAT(I, testing::NotNull());

  ASSERT_EQ(I->Value, 5);
  ASSERT_EQ(I->tokenLiteral(), "5");
}

TEST(ParserTests, testParsingPrefixExpressions) {
  const std::vector<std::tuple<std::string, std::string, int64_t>> Tests = {
      {"!5", "!", 5}, {"-15", "-", 15}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    ASSERT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    ASSERT_THAT(E, testing::NotNull());

    auto *PE = dynamic_cast<ast::PrefixExpression *>(E->Expr.get());
    ASSERT_THAT(PE, testing::NotNull());

    ASSERT_EQ(PE->Operator, std::get<1>(Test));

    testIntegerLiteral(PE->Right.get(), std::get<2>(Test));
  }

  const std::vector<std::tuple<std::string, std::string, bool>> BooleanTests = {
      {"!true", "!", true}, {"!false", "!", false}};

  for (const auto &Test : BooleanTests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    ASSERT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    ASSERT_THAT(E, testing::NotNull());

    auto *PE = dynamic_cast<ast::PrefixExpression *>(E->Expr.get());
    ASSERT_THAT(PE, testing::NotNull());

    ASSERT_EQ(PE->Operator, std::get<1>(Test));

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

    ASSERT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    ASSERT_THAT(E, testing::NotNull());

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

    ASSERT_EQ(Program->Statements.size(), 1);

    auto *E = dynamic_cast<ast::ExpressionStatement *>(
        Program->Statements.front().get());
    ASSERT_THAT(E, testing::NotNull());

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
      {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"},
      {"a * [1, 2, 3, 4][b * c] * d", "((a * ([1, 2, 3, 4][(b * c)])) * d)"},
      {"add(a * b[2], b[1], 2 * [1, 2][1])",
       "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"}};

  for (const auto &Test : Tests) {
    lexer::Lexer L(std::get<0>(Test));
    parser::Parser P(L);

    auto Program = P.parseProgram();
    checkParserErrors(P);

    auto Actual = Program->string();
    ASSERT_EQ(Actual, std::get<1>(Test));
  }
}

TEST(ParserTests, testIfExpression) {
  const std::string Input("if (x < y) { x }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  auto *IfE = dynamic_cast<ast::IfExpression *>(ES->Expr.get());
  ASSERT_THAT(IfE, testing::NotNull());

  testInfixExpression(IfE->Condition.get(), "x", "<", "y");

  ASSERT_EQ(IfE->Consequence->Statements.size(), 1);

  auto *Cons = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Consequence->Statements.front().get());
  ASSERT_THAT(Cons, testing::NotNull());

  testIdentifier(Cons->Expr.get(), "x");
  ASSERT_THAT(IfE->Alternative.get(), testing::IsNull());
}

TEST(ParserTests, testIfElseExpression) {
  const std::string Input("if (x < y) { x } else { y }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  auto *IfE = dynamic_cast<ast::IfExpression *>(ES->Expr.get());
  ASSERT_THAT(IfE, testing::NotNull());

  testInfixExpression(IfE->Condition.get(), "x", "<", "y");

  ASSERT_EQ(IfE->Consequence->Statements.size(), 1);

  auto *Cons = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Consequence->Statements.front().get());
  ASSERT_THAT(Cons, testing::NotNull());

  testIdentifier(Cons->Expr.get(), "x");

  ASSERT_EQ(IfE->Alternative->Statements.size(), 1);
  auto *Alt = dynamic_cast<ast::ExpressionStatement *>(
      IfE->Alternative->Statements.front().get());
  ASSERT_THAT(Alt, testing::NotNull());

  testIdentifier(Alt->Expr.get(), "y");
}

TEST(ParserTests, testFunctionLiteralParsing) {
  const std::string Input("fn(x, y) { x + y; }");

  lexer::Lexer L(Input);
  parser::Parser P(L);

  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  auto *Function = dynamic_cast<ast::FunctionLiteral *>(ES->Expr.get());
  ASSERT_THAT(Function, testing::NotNull());

  ASSERT_EQ(Function->Parameters.size(), 2);

  testLiteralExpression(Function->Parameters[0].get(), "x");
  testLiteralExpression(Function->Parameters[1].get(), "y");

  ASSERT_EQ(Function->Body->Statements.size(), 1);

  auto *Body = dynamic_cast<ast::ExpressionStatement *>(
      Function->Body->Statements.front().get());
  ASSERT_THAT(Body, testing::NotNull());

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
    ASSERT_THAT(Statement, testing::NotNull());

    auto *Function =
        dynamic_cast<ast::FunctionLiteral *>(Statement->Expr.get());
    ASSERT_THAT(Function, testing::NotNull());

    ASSERT_EQ(Function->Parameters.size(), std::get<1>(Test).size());

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

  ASSERT_EQ(Program->Statements.size(), 1);

  auto *ES = dynamic_cast<ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  auto *Call = dynamic_cast<ast::CallExpression *>(ES->Expr.get());
  ASSERT_THAT(Call, testing::NotNull());

  testIdentifier(Call->Function.get(), "add");

  ASSERT_EQ(Call->Arguments.size(), 3);

  testLiteralExpression(Call->Arguments[0].get(), (int64_t)1);
  testInfixExpression(Call->Arguments[1].get(), (int64_t)2, "*", (int64_t)3);
  testInfixExpression(Call->Arguments[2].get(), (int64_t)4, "+", (int64_t)5);
}

TEST(ParserTests, testStringLiteralExpression) {
  const std::string Input("\"hello world\";");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *StringLiteral = dynamic_cast<const ast::String *>(ES->Expr.get());
  ASSERT_THAT(StringLiteral, testing::NotNull());
  ASSERT_EQ(StringLiteral->Value, "hello world");
}

TEST(ParserTests, testParsingArrayLiterals) {
  const std::string Input("[1, 2 * 2, 3 + 3]");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *AL = dynamic_cast<const ast::ArrayLiteral *>(ES->Expr.get());
  ASSERT_THAT(AL, testing::NotNull());
  ASSERT_EQ(AL->Elements.size(), 3);
  testIntegerLiteral(AL->Elements.at(0).get(), 1);
  testInfixExpression(AL->Elements.at(1).get(), (int64_t)2, "*", (int64_t)2);
  testInfixExpression(AL->Elements.at(2).get(), (int64_t)3, "+", (int64_t)3);
}

TEST(ParserTests, testParsingIndexExpressions) {
  const std::string Input("myArray[1 + 1]");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *IE = dynamic_cast<const ast::IndexExpression *>(ES->Expr.get());
  ASSERT_THAT(IE, testing::NotNull());
  testIdentifier(IE->Left.get(), "myArray");
  testInfixExpression(IE->Index.get(), (int64_t)1, "+", (int64_t)1);
}

TEST(ParserTests, testParsingHashLiteralsStringKeys) {
  const std::string Input("{\"one\": 1, \"two\": 2, \"three\": 3}");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  ASSERT_EQ(Program->Statements.size(), 1);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *HL = dynamic_cast<const ast::HashLiteral *>(ES->Expr.get());
  ASSERT_THAT(HL, testing::NotNull());
  ASSERT_EQ(HL->Pairs.size(), 3);

  const std::vector<std::pair<std::string, int64_t>> Expected = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  for (const auto &P : HL->Pairs) {
    const auto *Key = dynamic_cast<const ast::String *>(P.first.get());
    ASSERT_THAT(Key, testing::NotNull());
    const auto &KeyVal = Key->Value;

    const auto Iter =
        std::find_if(Expected.begin(), Expected.end(),
                     [&KeyVal](const std::pair<std::string, int64_t> &E) {
                       return E.first == KeyVal;
                     });

    ASSERT_NE(Iter, Expected.end());
    ASSERT_EQ(Iter->first, KeyVal);
    testIntegerLiteral(P.second.get(), Iter->second);
  }
}

TEST(ParserTests, testParsingEmptyHashLiteral) {
  const std::string Input("{}");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *HL = dynamic_cast<const ast::HashLiteral *>(ES->Expr.get());
  ASSERT_THAT(HL, testing::NotNull());
  ASSERT_TRUE(HL->Pairs.empty());
}

TEST(ParserTests, testParsingHashLiteralsWithExpressions) {
  const std::string Input(
      "{\"one\": 0 + 1, \"two\": 10 - 8, \"three\": 15 / 5}");

  lexer::Lexer L(Input);
  parser::Parser P(L);
  auto Program = P.parseProgram();
  checkParserErrors(P);

  const auto *ES = dynamic_cast<const ast::ExpressionStatement *>(
      Program->Statements.front().get());
  ASSERT_THAT(ES, testing::NotNull());

  const auto *HL = dynamic_cast<const ast::HashLiteral *>(ES->Expr.get());
  ASSERT_THAT(HL, testing::NotNull());
  ASSERT_EQ(HL->Pairs.size(), 3);

  std::vector<std::pair<std::string, std::function<void(ast::Expression *)>>>
      TestFuncs = {{"one",
                    [](ast::Expression *Expr) {
                      testInfixExpression(Expr, (int64_t)0, "+", (int64_t)1);
                    }},
                   {"two",
                    [](ast::Expression *Expr) {
                      testInfixExpression(Expr, (int64_t)10, "-", (int64_t)8);
                    }},
                   {"three", [](ast::Expression *Expr) {
                      testInfixExpression(Expr, (int64_t)15, "/", (int64_t)5);
                    }}};

  for (const auto &P : HL->Pairs) {
    const auto *Key = dynamic_cast<const ast::String *>(P.first.get());
    ASSERT_THAT(Key, testing::NotNull());
    const auto &KeyVal = Key->Value;

    const auto Iter = std::find_if(
        TestFuncs.begin(), TestFuncs.end(),
        [&KeyVal](
            const std::pair<std::string, std::function<void(ast::Expression *)>>
                &F) { return F.first == KeyVal; });

    ASSERT_NE(Iter, TestFuncs.end());
    ASSERT_EQ(Iter->first, KeyVal);
    Iter->second(P.second.get());
  }
}

} // namespace monkey::parser::test
