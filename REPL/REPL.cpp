#include "REPL.h"

#include <Lexer/Lexer.h>

#include <iostream>

namespace monkey {

void REPL::start() {
  std::string Line;
  while (std::cin) {
    std::getline(std::cin, Line);
    std::cout << Line << std::endl;

    Lexer L(Line);

    for (Token Tok = L.nextToken(); Tok.Type != TokenType::END_OF_FILE;
         Tok = L.nextToken()) {
      std::cout << Tok << "\n";
    }
  }
}

} // namespace monkey
