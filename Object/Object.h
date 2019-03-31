#pragma once

#include "ObjectInterface.h"

#include <AST/AST.h>
#include <Environment/Environment.h>

#include <memory>

namespace monkey::object {

extern const std::string INTEGER_OBJ;
extern const std::string BOOLEAN_OBJ;
extern const std::string NULL_OBJ;
extern const std::string RETURN_VALUE_OBJ;
extern const std::string ERROR_OBJ;
extern const std::string FUNCTION_OBJ;
extern const std::string STRING_OBJ;
extern const std::string BUILTIN_OBJ;
extern const std::string ARRAY_OBJ;

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
  explicit ReturnValue(std::shared_ptr<object::Object>);
  virtual ~ReturnValue() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  std::shared_ptr<object::Object> Value;
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

struct Function : public Object {
  Function(std::vector<std::unique_ptr<ast::Identifier>> &&,
           std::unique_ptr<ast::BlockStatement>, environment::Environment &);
  virtual ~Function() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  std::vector<std::unique_ptr<ast::Identifier>> Parameters;
  std::unique_ptr<ast::BlockStatement> Body;
  environment::Environment Env;
};

struct String : public Object {
  template <typename T>
  explicit String(T &&Value) : Value(std::forward<T>(Value)) {}
  virtual ~String() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  const std::string Value;
};

using BuiltInFunction = std::function<std::shared_ptr<Object>(
    const std::vector<std::shared_ptr<Object>> &)>;

struct BuiltIn : public Object {
  explicit BuiltIn(const BuiltInFunction &);
  virtual ~BuiltIn() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  BuiltInFunction Fn;
};

struct Array : public Object {
  Array(std::vector<std::shared_ptr<object::Object>> &&);
  virtual ~Array() = default;

  // Object impl.
  const ObjectType &type() const override;
  std::string inspect() const override;

  const std::vector<std::shared_ptr<object::Object>> Elements;
};

} // namespace monkey::object
