#pragma once

#include "ObjectInterface.h"

#include <AST/AST.h>
#include <Code/Code.h>
#include <Environment/Environment.h>

#include <boost/pool/pool_alloc.hpp>

#include <functional>
#include <memory>

namespace monkey::object {

extern const std::shared_ptr<Object> TrueGlobal;
extern const std::shared_ptr<Object> FalseGlobal;
extern const std::shared_ptr<Object> NullGlobal;

const std::shared_ptr<Object> &nativeBooleanToBooleanObject(bool Val);

enum class ObjectType : uint64_t {
  INTEGER_OBJ,
  BOOLEAN_OBJ,
  NULL_OBJ,
  RETURN_VALUE_OBJ,
  ERROR_OBJ,
  FUNCTION_OBJ,
  STRING_OBJ,
  BUILTIN_OBJ,
  ARRAY_OBJ,
  HASH_OBJ,
  COMPILED_FUNCTION_OBJ,
  CLOSURE_OBJ
};

const char *objTypeToString(ObjectType);

struct Integer : public Object {
  explicit Integer(int64_t);
  virtual ~Integer() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;
  size_t hash() const override;
  bool equals(const Object &) const override;

  int64_t Value;
};

struct Boolean : public Object {
  explicit Boolean(bool);
  virtual ~Boolean() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;
  size_t hash() const override;
  bool equals(const Object &) const override;

  bool Value;
};

struct Null : public Object {
  Null() = default;
  virtual ~Null() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;
};

struct ReturnValue : public Object {
  template <typename T>
  explicit ReturnValue(T &&Value) : Value(std::forward<T>(Value)) {}
  virtual ~ReturnValue() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  std::shared_ptr<object::Object> Value;
};

struct Error : public Object {
  template <typename T>
  explicit Error(T &&Message) : Message(std::forward<T>(Message)) {}
  virtual ~Error() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  const std::string Message;
};

struct Function : public Object {
  Function(std::vector<std::unique_ptr<ast::Identifier>> &&,
           std::unique_ptr<ast::BlockStatement>,
           std::shared_ptr<environment::Environment> &);
  virtual ~Function() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  std::vector<std::unique_ptr<ast::Identifier>> Parameters;
  std::unique_ptr<ast::BlockStatement> Body;
  std::shared_ptr<environment::Environment> Env;
};

struct String : public Object {
  template <typename T>
  explicit String(T &&Value) : Value(std::forward<T>(Value)) {}
  virtual ~String() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;
  size_t hash() const override;
  bool equals(const Object &) const override;

  const std::string Value;
};

using BuiltInFunction = std::function<std::shared_ptr<Object>(
    const std::vector<std::shared_ptr<Object>> &)>;

struct BuiltIn : public Object {
  explicit BuiltIn(const BuiltInFunction &);
  virtual ~BuiltIn() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  BuiltInFunction Fn;
};

struct Array : public Object {
  explicit Array(std::vector<std::shared_ptr<object::Object>> &&);
  virtual ~Array() = default;

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  const std::vector<std::shared_ptr<object::Object>> Elements;
};

struct HashKey {
  explicit HashKey(const std::shared_ptr<object::Object> &);

  bool operator==(const HashKey &) const;

  const std::shared_ptr<object::Object> Key;
};

bool hasHashKey(const HashKey &);

struct HashKeyHasher {
  size_t operator()(const HashKey &) const;
};

struct Hash : public Object {
  explicit Hash(std::unordered_map<HashKey, std::shared_ptr<object::Object>,
                                   HashKeyHasher> &&);

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  const std::unordered_map<HashKey, std::shared_ptr<object::Object>,
                           HashKeyHasher>
      Pairs;
};

struct CompiledFunction : public Object {
  template <typename T>
  CompiledFunction(T &&Ins, int NumLocals, int NumParameters)
      : Ins(std::forward<T>(Ins)), NumLocals(NumLocals),
        NumParameters(NumParameters) {}

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  code::Instructions Ins;
  const int NumLocals;
  const int NumParameters;
};

struct Closure : public Object {
  template <typename T> explicit Closure(T &&Fn) : Fn(std::forward<T>(Fn)) {}
  template <typename T0, typename T1>
  Closure(T0 &&Fn, T1 &&Free)
      : Fn(std::forward<T0>(Fn)), Free(std::forward<T1>(Free)) {}

  // Object impl.
  ObjectType type() const override;
  std::string inspect() const override;

  const std::shared_ptr<Object> Fn;
  const std::vector<std::shared_ptr<Object>> Free;
};

template <typename T, ObjectType ObjType>
inline T objCastImpl(const Object *Obj) {
  if (Obj && Obj->type() == ObjType)
    return static_cast<T>(Obj);
  else
    return nullptr;
}

template <typename T> inline T objCast(const Object *) {
  static_assert(sizeof(T) != sizeof(T),
                "objCast must be specialised for this type");
}

template <> inline const Integer *objCast<const Integer *>(const Object *Obj) {
  return objCastImpl<const Integer *, ObjectType::INTEGER_OBJ>(Obj);
}

template <> inline const Boolean *objCast<const Boolean *>(const Object *Obj) {
  return objCastImpl<const Boolean *, ObjectType::BOOLEAN_OBJ>(Obj);
}

template <> inline const Null *objCast<const Null *>(const Object *Obj) {
  return objCastImpl<const Null *, ObjectType::NULL_OBJ>(Obj);
}

template <> inline ReturnValue *objCast<ReturnValue *>(const Object *Obj) {
  return const_cast<ReturnValue *>(
      objCastImpl<const ReturnValue *, ObjectType::RETURN_VALUE_OBJ>(Obj));
}

template <> inline const Error *objCast<const Error *>(const Object *Obj) {
  return objCastImpl<const Error *, ObjectType::ERROR_OBJ>(Obj);
}

template <>
inline const Function *objCast<const Function *>(const Object *Obj) {
  return objCastImpl<const Function *, ObjectType::FUNCTION_OBJ>(Obj);
}

template <> inline const String *objCast<const String *>(const Object *Obj) {
  return objCastImpl<const String *, ObjectType::STRING_OBJ>(Obj);
}

template <> inline const BuiltIn *objCast<const BuiltIn *>(const Object *Obj) {
  return objCastImpl<const BuiltIn *, ObjectType::BUILTIN_OBJ>(Obj);
}

template <> inline const Array *objCast<const Array *>(const Object *Obj) {
  return objCastImpl<const Array *, ObjectType::ARRAY_OBJ>(Obj);
}

template <> inline const Hash *objCast<const Hash *>(const Object *Obj) {
  return objCastImpl<const Hash *, ObjectType::HASH_OBJ>(Obj);
}

template <>
inline const CompiledFunction *
objCast<const CompiledFunction *>(const Object *Obj) {
  return objCastImpl<const CompiledFunction *,
                     ObjectType::COMPILED_FUNCTION_OBJ>(Obj);
}

template <> inline const Closure *objCast<const Closure *>(const Object *Obj) {
  return objCastImpl<const Closure *, ObjectType::CLOSURE_OBJ>(Obj);
}

extern boost::pool_allocator<object::Integer> IntegerAlloc;
extern boost::pool_allocator<object::ReturnValue> ReturnAlloc;
extern boost::pool_allocator<object::Function> FunctionAlloc;
extern boost::pool_allocator<object::String> StringAlloc;
extern boost::pool_allocator<object::Array> ArrayAlloc;
extern boost::pool_allocator<object::Hash> HashAlloc;
extern boost::pool_allocator<object::Closure> ClosureAlloc;

inline std::shared_ptr<Integer> makeInteger(int Value) {
  return std::allocate_shared<Integer, boost::pool_allocator<Integer>>(
      IntegerAlloc, Value);
}

template <typename T>
inline std::shared_ptr<ReturnValue> makeReturn(T &&Return) {
  return std::allocate_shared<ReturnValue, boost::pool_allocator<ReturnValue>>(
      ReturnAlloc, std::forward<T>(Return));
}

inline std::shared_ptr<Function>
makeFunction(std::vector<std::unique_ptr<ast::Identifier>> &&Parameters,
             std::unique_ptr<ast::BlockStatement> Body,
             std::shared_ptr<environment::Environment> &Env) {
  return std::allocate_shared<Function, boost::pool_allocator<Function>>(
      FunctionAlloc, std::move(Parameters), std::move(Body), Env);
}

template <typename T> inline std::shared_ptr<String> makeString(T &&Value) {
  return std::allocate_shared<String, boost::pool_allocator<String>>(
      StringAlloc, std::forward<T>(Value));
}

inline std::shared_ptr<Array>
makeArray(std::vector<std::shared_ptr<Object>> &&Value) {
  return std::allocate_shared<Array, boost::pool_allocator<Array>>(
      ArrayAlloc, std::move(Value));
}

inline std::shared_ptr<Hash> makeHash(
    std::unordered_map<HashKey, std::shared_ptr<object::Object>, HashKeyHasher>
        &&Value) {
  return std::allocate_shared<Hash, boost::pool_allocator<Hash>>(
      HashAlloc, std::move(Value));
}

template <typename T> inline std::shared_ptr<Closure> makeClosure(T &&Value) {
  return std::allocate_shared<Closure, boost::pool_allocator<Closure>>(
      ClosureAlloc, std::forward<T>(Value));
}

template <typename T0, typename T1>
inline std::shared_ptr<Closure> makeClosure(T0 &&Fn, T1 &&Free) {
  return std::allocate_shared<Closure, boost::pool_allocator<Closure>>(
      ClosureAlloc, std::forward<T0>(Fn), std::forward<T1>(Free));
}

} // namespace monkey::object
