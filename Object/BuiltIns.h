#pragma once

#include "Object.h"

namespace monkey::object {

extern const std::vector<std::pair<std::string, std::shared_ptr<BuiltIn>>>
    BUILTINS;

std::shared_ptr<Error> newError(const char *, ...);

} // namespace monkey::object
