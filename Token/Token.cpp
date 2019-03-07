#include "Token.h"

#include <algorithm>
#include <vector>

namespace monkey {

namespace {

const std::vector<std::pair<std::string, TokenType>> Keywords = {
    {"fn", TokenType::FUNCTION},  {"let", TokenType::LET},
    {"true", TokenType::TRUE},    {"false", TokenType::FALSE},
    {"if", TokenType::IF},        {"else", TokenType::ELSE},
    {"return", TokenType::RETURN}};

} // namespace

TokenType lookupIdentifier(const std::string &Identifier) {
  const auto Iter =
      std::find_if(Keywords.begin(), Keywords.end(),
                   [&Identifier](const std::pair<std::string, TokenType> &K) {
                     return K.first == Identifier;
                   });

  if (Iter != Keywords.end()) {
    return Iter->second;
  }

  return TokenType::IDENT;
}

const char *tokenTypeToString(TokenType Type) {
  switch (Type) {
  case TokenType::ILLEGAL:
    return "ILLEGAL";
  case TokenType::END_OF_FILE:
    return "EOF";
  case TokenType::IDENT:
    return "IDENT";
  case TokenType::INT:
    return "INT";
  case TokenType::ASSIGN:
    return "=";
  case TokenType::PLUS:
    return "+";
  case TokenType::MINUS:
    return "-";
  case TokenType::BANG:
    return "!";
  case TokenType::ASTERISK:
    return "*";
  case TokenType::SLASH:
    return "/";
  case TokenType::LT:
    return "<";
  case TokenType::GT:
    return ">";
  case TokenType::EQ:
    return "==";
  case TokenType::NOT_EQ:
    return "!=";
  case TokenType::COMMA:
    return ",";
  case TokenType::SEMICOLON:
    return ";";
  case TokenType::LPAREN:
    return "(";
  case TokenType::RPAREN:
    return ")";
  case TokenType::LBRACE:
    return "{";
  case TokenType::RBRACE:
    return "}";
  case TokenType::FUNCTION:
    return "FUNCTION";
  case TokenType::LET:
    return "LET";
  case TokenType::TRUE:
    return "TRUE";
  case TokenType::FALSE:
    return "FALSE";
  case TokenType::IF:
    return "IF";
  case TokenType::ELSE:
    return "ELSE";
  case TokenType::RETURN:
    return "RETURN";
  }

  return "UNKNOWN";
}

} // namespace monkey
