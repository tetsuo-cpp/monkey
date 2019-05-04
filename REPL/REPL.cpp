#include "REPL.h"

#include <Compiler/Compiler.h>
#include <Lexer/Lexer.h>
#include <VM/VM.h>

#include <iostream>

namespace {

const std::string Prompt(">> ");

} // namespace

namespace monkey::repl {

void REPL::start() {
  std::string Line;
  environment::Environment Env;
  compiler::SymbolTable ST;
  std::vector<std::shared_ptr<object::Object>> Constants;
  std::array<std::shared_ptr<object::Object>, GlobalsSize> Globals;

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

    compiler::Compiler C(ST, Constants);
    try {
      C.compile(Program.get());
    } catch (const std::runtime_error &E) {
      std::cout << "Woops! Compilation failed:\n  " << E.what() << "\n";
    }

    vm::VM Machine(C.byteCode(), Globals);
    try {
      Machine.run();
    } catch (const std::runtime_error &E) {
      std::cout << "Woops! Compilation failed:\n  " << E.what() << "\n";
    }

    const auto *StackTop = Machine.lastPoppedStackElem();
    if (StackTop)
      std::cout << StackTop->inspect() << "\n";
  }
}

void REPL::printParserErrors(const parser::Parser &P) const {
  std::cout << "Woops! We ran into some Monkey business here.\n";
  std::cout << " parser errors:\n";

  for (const auto &Error : P.errors())
    std::cout << "\t" << Error << "\n";
}

} // namespace monkey::repl
