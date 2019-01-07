#pragma once

#include <string>

namespace monkey {

enum class TokenType {
  ILLEGAL,
  END_OF_FILE,
  IDENT,
  INT,
  ASSIGN,
  PLUS,
  COMMA,
  SEMICOLON,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  FUNCTION,
  LET
};

struct Token {
  Token() = default;
  template <typename T>
  Token(TokenType Type, T &&Literal)
      : Type(Type), Literal(std::forward<T>(Literal)) {}
  Token(TokenType Type, char Literal) : Type(Type), Literal(1, Literal) {}

  TokenType Type;
  std::string Literal;
};

} // namespace monkey
