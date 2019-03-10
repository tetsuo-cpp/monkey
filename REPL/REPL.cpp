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
    Parser P(L);

    auto Program = P.parseProgram();
    if (!P.errors().empty()) {
      printParserErrors(P);
      continue;
    }

    std::cout << Program->string() << "\n";
  }
}

void REPL::printParserErrors(const Parser &P) const {
  std::cout << "Woops! We ran into some Monkey business here.\n";
  std::cout << " parser errors:\n";

  for (const auto &Error : P.errors()) {
    std::cout << "\t" << Error << "\n";
  }
}

} // namespace monkey
