#pragma once

namespace monkey {

class REPL {
public:
  virtual ~REPL() = default;

  void start();
};

} // namespace monkey
