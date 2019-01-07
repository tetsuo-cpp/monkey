#pragma once

#include <Token/Token.h>

namespace monkey {

class Lexer {
public:
  Lexer(const std::string &Input);
  virtual ~Lexer() = default;

  Token nextToken();

private:
  void readChar();

  const std::string &Input;
  unsigned int Position;
  unsigned int ReadPosition;
  char Current;
};

} // namespace monkey
