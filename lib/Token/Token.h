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
  TokenType Type;
  std::string Literal;
};

} // namespace monkey
