#pragma once

#include <string>

namespace monkey {

enum class TokenType {
  ILLEGAL,
  END_OF_FILE,
  IDENT,
  INT,
  // Operators.
  ASSIGN,
  PLUS,
  MINUS,
  BANG,
  ASTERISK,
  SLASH,
  LT,
  GT,
  EQ,
  NOT_EQ,
  COMMA,
  SEMICOLON,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  // Keywords.
  FUNCTION,
  LET,
  TRUE,
  FALSE,
  IF,
  ELSE,
  RETURN,
  STRING
};

const char *tokenTypeToString(TokenType Type);

struct Token {
  Token() = default;
  template <typename T>
  Token(TokenType Type, T &&Literal)
      : Type(Type), Literal(std::forward<T>(Literal)) {}
  Token(TokenType Type, char Literal) : Type(Type), Literal(1, Literal) {}

  template <typename T> friend T &operator<<(T &Stream, const Token &Tok) {
    Stream << "{Type=" << tokenTypeToString(Tok.Type)
           << ", Literal=" << Tok.Literal << "}";
    return Stream;
  }

  TokenType Type;
  std::string Literal;
};

TokenType lookupIdentifier(const std::string &Identifier);

} // namespace monkey
