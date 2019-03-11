#include "Lexer.h"

#include <Token/Token.h>

namespace {

bool isLetter(char C) { return std::isalpha(C) || C == '_'; }

} // namespace

namespace monkey::lexer {

Lexer::Lexer(const std::string &Input)
    : Input(Input), Position(0), ReadPosition(0), Current(0) {
  readChar();
}

Token Lexer::nextToken() {
  skipWhitespace();

  Token Tok;
  if (Position >= Input.size()) {
    Tok = Token(TokenType::END_OF_FILE, static_cast<char>(0));
    return Tok;
  }

  switch (Current) {
  case '=':
    if (peekChar() == '=') {
      readChar();
      Tok = Token(TokenType::EQ, "==");
    } else {
      Tok = Token(TokenType::ASSIGN, Current);
    }

    break;
  case '+':
    Tok = Token(TokenType::PLUS, Current);
    break;
  case '-':
    Tok = Token(TokenType::MINUS, Current);
    break;
  case '!':
    if (peekChar() == '=') {
      readChar();
      Tok = Token(TokenType::NOT_EQ, "!=");
    } else {
      Tok = Token(TokenType::BANG, Current);
    }

    break;
  case '/':
    Tok = Token(TokenType::SLASH, Current);
    break;
  case '*':
    Tok = Token(TokenType::ASTERISK, Current);
    break;
  case '<':
    Tok = Token(TokenType::LT, Current);
    break;
  case '>':
    Tok = Token(TokenType::GT, Current);
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
  case '{':
    Tok = Token(TokenType::LBRACE, Current);
    break;
  case '}':
    Tok = Token(TokenType::RBRACE, Current);
    break;
  default:
    if (isLetter(Current)) {
      Tok.Literal = readIdentifier();
      Tok.Type = lookupIdentifier(Tok.Literal);
      return Tok;
    } else if (std::isdigit(Current)) {
      Tok.Type = TokenType::INT;
      Tok.Literal = readNumber();
      return Tok;
    }

    Tok.Type = TokenType::ILLEGAL;
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

char Lexer::peekChar() const {
  if (ReadPosition >= Input.size()) {
    return 0;
  } else {
    return Input.at(ReadPosition);
  }
}

std::string Lexer::readIdentifier() {
  std::string Identifier;
  while (isLetter(Current)) {
    Identifier.push_back(Current);
    readChar();
  }

  return Identifier;
}

std::string Lexer::readNumber() {
  std::string Number;
  while (std::isdigit(Current)) {
    Number.push_back(Current);
    readChar();
  }

  return Number;
}

void Lexer::skipWhitespace() {
  while (std::isspace(Current)) {
    readChar();
  }
}

} // namespace monkey::lexer
