#include "AST.h"

namespace {

const std::string Empty;

} // namespace

namespace monkey {

const std::string &Program::tokenLiteral() const {
  if (Statements.empty()) {
    return Statements.front()->tokenLiteral();
  } else {
    return Empty;
  }
}

const std::string &Identifier::tokenLiteral() const { return Token.Literal; }

const std::string &LetStatement::tokenLiteral() const { return Token.Literal; }

} // namespace monkey
