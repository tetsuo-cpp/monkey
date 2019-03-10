#pragma once

#include <Parser/Parser.h>

namespace monkey {

class REPL {
public:
  virtual ~REPL() = default;

  void start();

private:
  void printParserErrors(const Parser &) const;
};

} // namespace monkey
