#include "Compiler.h"

namespace monkey::compiler {

std::string Compiler::compile(const ast::Node *Node) {
  static_cast<void>(Node);
  return std::string();
}

ByteCode Compiler::byteCode() const {
  return ByteCode(std::move(Instructions), std::move(Constants));
}

} // namespace monkey::compiler
