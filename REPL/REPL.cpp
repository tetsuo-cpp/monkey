#include "REPL.h"

#include <Evaluator/Evaluator.h>
#include <Lexer/Lexer.h>

#include <iostream>

namespace {

const std::string Prompt(">> ");

} // namespace

namespace monkey::repl {

void REPL::start() {
  std::string Line;
  while (std::cin) {
    std::cout << Prompt;
    std::getline(std::cin, Line);

    lexer::Lexer L(Line);
    parser::Parser P(L);

    auto Program = P.parseProgram();
    if (!P.errors().empty()) {
      printParserErrors(P);
      continue;
    }

    auto Evaluated = evaluator::eval(Program.get());
    if (Evaluated) {
      std::cout << Evaluated->inspect() << "\n";
    }
  }
}

void REPL::printParserErrors(const parser::Parser &P) const {
  std::cout << "Woops! We ran into some Monkey business here.\n";
  std::cout << " parser errors:\n";

  for (const auto &Error : P.errors()) {
    std::cout << "\t" << Error << "\n";
  }
}

} // namespace monkey::repl
