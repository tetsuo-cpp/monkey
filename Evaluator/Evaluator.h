#pragma once

#include <AST/AST.h>
#include <Object/Object.h>

namespace monkey::evaluator {

std::unique_ptr<object::Object> eval(const ast::Node *);

} // namespace monkey::evaluator
