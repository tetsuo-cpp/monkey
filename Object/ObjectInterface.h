#pragma once

#include <string>

namespace monkey::object {

using ObjectType = std::string;

struct Object {
  virtual ~Object() = default;

  virtual const ObjectType &type() const = 0;
  virtual std::string inspect() const = 0;
  virtual size_t hash() const { throw std::runtime_error("no impl"); }
};

} // namespace monkey::object
