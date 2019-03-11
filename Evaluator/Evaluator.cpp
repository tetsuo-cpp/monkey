#include "Evaluator.h"

namespace monkey::evaluator {

namespace {

std::unique_ptr<object::Object>
evalStatements(const std::vector<std::unique_ptr<ast::Statement>> &Statements) {
  std::unique_ptr<object::Object> Result;
  for (const auto &Statement : Statements) {
    Result = eval(Statement.get());
  }

  return Result;
}

} // namespace

// TODO: Maybe switch to using a visitor to avoid the constant dynamic casting?
std::unique_ptr<object::Object> eval(const ast::Node *Node) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program) {
    return evalStatements(Program->Statements);
  }

  const auto *ExprS = dynamic_cast<const ast::ExpressionStatement *>(Node);
  if (ExprS) {
    return eval(ExprS->Expr.get());
  }

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL) {
    return std::make_unique<object::Integer>(IntegerL->Value);
  }

  return nullptr;
}

} // namespace monkey::evaluator
