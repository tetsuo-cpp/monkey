#include <Token/Token.h>

#include <gtest/gtest.h>

namespace monkey::test {

class Lexer {
public:
  virtual ~Lexer() = default;

  Token nextToken() { return Token{TokenType::END_OF_FILE, std::string()}; }
};

TEST(LexerTests, TestNextToken) {
  const std::string Input("=+(){},;");

  const std::unordered_map<TokenType, std::string> Tests{
      {TokenType::ASSIGN, "="},    {TokenType::PLUS, "+"},
      {TokenType::LPAREN, "("},    {TokenType::RPAREN, ")"},
      {TokenType::LBRACE, "{"},    {TokenType::RBRACE, "}"},
      {TokenType::COMMA, ","},     {TokenType::SEMICOLON, ";"},
      {TokenType::END_OF_FILE, ""}};

  Lexer L;

  for (const auto &Pair : Tests) {
    const auto Tok = L.nextToken();

    EXPECT_EQ(Tok.Type, Pair.first);
    EXPECT_EQ(Tok.Literal, Pair.second);
  }
}

} // namespace monkey::test
