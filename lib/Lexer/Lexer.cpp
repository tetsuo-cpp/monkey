#include "Lexer.h"

namespace monkey {

Lexer::Lexer(const std::string &Input)
    : Input(Input), Position(0), ReadPosition(0), Current(0) {
  readChar();
}

Token Lexer::nextToken() {
  Token Tok;
  switch (Current) {
  case '=':
    Tok = Token(TokenType::ASSIGN, Current);
    break;
  case ';':
    Tok = Token(TokenType::SEMICOLON, Current);
    break;
  case '(':
    Tok = Token(TokenType::LPAREN, Current);
    break;
  case ')':
    Tok = Token(TokenType::RPAREN, Current);
    break;
  case ',':
    Tok = Token(TokenType::COMMA, Current);
    break;
  case '+':
    Tok = Token(TokenType::PLUS, Current);
    break;
  case '{':
    Tok = Token(TokenType::LBRACE, Current);
    break;
  case '}':
    Tok = Token(TokenType::RBRACE, Current);
    break;
  default:
    Tok.Type = TokenType::END_OF_FILE;
    break;
  }

  readChar();
  return Tok;
}

void Lexer::readChar() {
  if (ReadPosition >= Input.size()) {
    Current = 0;
  } else {
    Current = Input.at(ReadPosition);
  }

  Position = ReadPosition;
  ++ReadPosition;
}

} // namespace monkey
