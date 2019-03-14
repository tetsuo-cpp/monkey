#pragma once

#include <memory>
#include <string>

namespace monkey::object {

extern const std::string INTEGER_OBJ;
extern const std::string BOOLEAN_OBJ;
extern const std::string NULL_OBJ;
extern const std::string RETURN_VALUE_OBJ;
extern const std::string ERROR_OBJ;

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

struct ReturnValue : public Object {
  explicit ReturnValue(std::unique_ptr<object::Object>);
  virtual ~ReturnValue() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  std::unique_ptr<object::Object> Value;
};

struct Error : public Object {
  template <typename T>
  explicit Error(T &&Message) : Message(std::forward<T>(Message)) {}
  virtual ~Error() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  const std::string Message;
};

} // namespace monkey::object
