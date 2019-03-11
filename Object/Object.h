#pragma once

#include <string>

namespace monkey::object {

using ObjectType = std::string;

struct Object {
  virtual ~Object() = default;

  virtual const ObjectType &type() const = 0;
  virtual std::string inspect() const = 0;
};

struct Integer : public Object {
  explicit Integer(int64_t);
  virtual ~Integer() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  int64_t Value;
};

struct Boolean : public Object {
  explicit Boolean(bool);
  virtual ~Boolean() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  bool Value;
};

struct Null : public Object {
  Null() = default;
  virtual ~Null() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;
};

} // namespace monkey::object
