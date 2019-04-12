#pragma once

#include <stdexcept>
#include <string>

namespace monkey::object {

enum class ObjectType : uint64_t;

struct Object {
  virtual ~Object() = default;

  virtual ObjectType type() const = 0;
  virtual std::string inspect() const = 0;
  virtual size_t hash() const { throw std::runtime_error("no impl"); }
  virtual bool equals(const Object &) const {
    throw std::runtime_error("no impl");
  }
};

} // namespace monkey::object
