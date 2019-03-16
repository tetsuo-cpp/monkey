#pragma once

#include <string>

namespace monkey::object {

using ObjectType = std::string;

struct Object {
  virtual ~Object() = default;

  virtual const ObjectType &type() const = 0;
  virtual std::string inspect() const = 0;
};

} // namespace monkey::object
