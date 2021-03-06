#pragma once

#include <AST/AST.h>
#include <Environment/Environment.h>
#include <Object/Object.h>

namespace monkey::evaluator {

std::shared_ptr<object::Object>
eval(ast::Node *, std::shared_ptr<environment::Environment> &);

} // namespace monkey::evaluator
