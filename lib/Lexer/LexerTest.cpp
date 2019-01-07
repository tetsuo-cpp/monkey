#include <Lexer/Lexer.h>
#include <Token/Token.h>

#include <gtest/gtest.h>

namespace monkey::test {

TEST(LexerTests, TestNextToken) {
  const std::string Input("=+(){},;");

  const std::vector<std::pair<TokenType, std::string>> Tests{
      {TokenType::ASSIGN, "="},    {TokenType::PLUS, "+"},
      {TokenType::LPAREN, "("},    {TokenType::RPAREN, ")"},
      {TokenType::LBRACE, "{"},    {TokenType::RBRACE, "}"},
      {TokenType::COMMA, ","},     {TokenType::SEMICOLON, ";"},
      {TokenType::END_OF_FILE, ""}};

  Lexer L(Input);

  for (const auto &Pair : Tests) {
    const auto Tok = L.nextToken();

    EXPECT_EQ(Tok.Type, Pair.first);
    EXPECT_EQ(Tok.Literal, Pair.second);
  }
}

} // namespace monkey::test
