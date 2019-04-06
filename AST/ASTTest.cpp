#include "AST.h"

#include <Token/Token.h>

#include <gtest/gtest.h>

#include <memory>

namespace monkey::ast::test {

TEST(ASTTests, testString) {
  std::vector<std::unique_ptr<Statement>> Statements;
  Statements.push_back(std::make_unique<LetStatement>(
      Token(TokenType::LET, "let"),
      std::make_unique<Identifier>(Token(TokenType::IDENT, "myVar"), "myVar"),
      std::make_unique<Identifier>(Token(TokenType::IDENT, "anotherVar"),
                                   "anotherVar")));

  auto P = std::make_unique<Program>(std::move(Statements));
  ASSERT_EQ(P->string(), "let myVar = anotherVar;");
}

} // namespace monkey::ast::test
