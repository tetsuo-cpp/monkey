#pragma once

#include <Parser/Parser.h>

namespace monkey::repl {

class REPL {
public:
  virtual ~REPL() = default;

  void start();

private:
  void printParserErrors(const parser::Parser &) const;
};

} // namespace monkey::repl
