#pragma once

#include <Token/Token.h>

namespace monkey::lexer {

class Lexer {
public:
  explicit Lexer(const std::string &Input);
  virtual ~Lexer() = default;

  Token nextToken();

private:
  void readChar();
  char peekChar() const;
  std::string readIdentifier();
  std::string readNumber();
  void skipWhitespace();
  std::string readString();

  const std::string &Input;
  unsigned int Position;
  unsigned int ReadPosition;
  char Current;
};

} // namespace monkey::lexer
