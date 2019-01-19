#include "REPL.h"

#include <Lexer/Lexer.h>

#include <iostream>

namespace {

const std::string Prompt(">> ");

} // namespace

namespace monkey {

void REPL::start() {
  std::string Line;
  while (std::cin) {
    std::cout << Prompt;
    std::getline(std::cin, Line);

    Lexer L(Line);

    for (Token Tok = L.nextToken(); Tok.Type != TokenType::END_OF_FILE;
         Tok = L.nextToken()) {
      std::cout << Tok << "\n";
    }
  }
}

} // namespace monkey
