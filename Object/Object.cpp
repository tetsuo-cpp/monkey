#include "Object.h"

#include <sstream>

namespace monkey::object {

const std::shared_ptr<Object> TrueGlobal = std::make_shared<Boolean>(true);
const std::shared_ptr<Object> FalseGlobal = std::make_shared<Boolean>(false);
const std::shared_ptr<Object> NullGlobal = std::make_shared<Null>();

boost::pool_allocator<object::Integer> IntegerAlloc;
boost::pool_allocator<object::ReturnValue> ReturnAlloc;
boost::pool_allocator<object::Function> FunctionAlloc;
boost::pool_allocator<object::String> StringAlloc;
boost::pool_allocator<object::Array> ArrayAlloc;
boost::pool_allocator<object::Hash> HashAlloc;
boost::pool_allocator<object::Closure> ClosureAlloc;

const std::shared_ptr<Object> &nativeBooleanToBooleanObject(bool Val) {
  if (Val)
    return TrueGlobal;

  return FalseGlobal;
}

const char *objTypeToString(ObjectType Type) {
  switch (Type) {
  case ObjectType::INTEGER_OBJ:
    return "INTEGER";
  case ObjectType::BOOLEAN_OBJ:
    return "BOOLEAN";
  case ObjectType::NULL_OBJ:
    return "NULL";
  case ObjectType::RETURN_VALUE_OBJ:
    return "RETURN_VALUE";
  case ObjectType::ERROR_OBJ:
    return "ERROR";
  case ObjectType::FUNCTION_OBJ:
    return "FUNCTION";
  case ObjectType::STRING_OBJ:
    return "STRING";
  case ObjectType::BUILTIN_OBJ:
    return "BUILTIN";
  case ObjectType::ARRAY_OBJ:
    return "ARRAY";
  case ObjectType::HASH_OBJ:
    return "HASH";
  case ObjectType::COMPILED_FUNCTION_OBJ:
    return "COMPILED_FUNCTION";
  case ObjectType::CLOSURE_OBJ:
    return "CLOSURE";
  }

  return "UNKNOWN";
}

Integer::Integer(int64_t Value) : Value(Value) {}

ObjectType Integer::type() const { return ObjectType::INTEGER_OBJ; }

std::string Integer::inspect() const { return std::to_string(Value); }

size_t Integer::hash() const { return std::hash<int64_t>()(Value); }

bool Integer::equals(const Object &Obj) const {
  const auto *I = objCast<const Integer *>(&Obj);
  if (!I)
    return false;

  return Value == I->Value;
}

Boolean::Boolean(bool Value) : Value(Value) {}

ObjectType Boolean::type() const { return ObjectType::BOOLEAN_OBJ; }

std::string Boolean::inspect() const { return Value ? "true" : "false"; }

size_t Boolean::hash() const { return std::hash<bool>()(Value); }

bool Boolean::equals(const Object &Obj) const {
  const auto *B = objCast<const Boolean *>(&Obj);
  if (!B)
    return false;

  return Value == B->Value;
}

ObjectType Null::type() const { return ObjectType::NULL_OBJ; }

std::string Null::inspect() const { return "null"; }

ObjectType ReturnValue::type() const { return ObjectType::RETURN_VALUE_OBJ; }

std::string ReturnValue::inspect() const { return Value->inspect(); }

ObjectType Error::type() const { return ObjectType::ERROR_OBJ; }

std::string Error::inspect() const { return "ERROR: " + Message; }

Function::Function(std::vector<std::unique_ptr<ast::Identifier>> &&Parameters,
                   std::unique_ptr<ast::BlockStatement> Body,
                   std::shared_ptr<environment::Environment> &Env)
    : Parameters(std::move(Parameters)), Body(std::move(Body)), Env(Env) {}

ObjectType Function::type() const { return ObjectType::FUNCTION_OBJ; }

std::string Function::inspect() const {
  std::stringstream SS;
  SS << "fn(";
  for (const auto &Param : Parameters) {
    SS << Param->string();
    if (&Param != &Parameters.back())
      SS << ", ";
  }

  SS << ") {\n";
  SS << Body->string();
  SS << "\n}";
  return SS.str();
}

ObjectType String::type() const { return ObjectType::STRING_OBJ; }

std::string String::inspect() const { return Value; }

size_t String::hash() const { return std::hash<std::string>()(Value); }

bool String::equals(const Object &Obj) const {
  const auto *S = objCast<const String *>(&Obj);
  if (!S)
    return false;

  return Value == S->Value;
}

BuiltIn::BuiltIn(const BuiltInFunction &Fn) : Fn(Fn) {}

ObjectType BuiltIn::type() const { return ObjectType::BUILTIN_OBJ; }

std::string BuiltIn::inspect() const { return "builtin string"; }

Array::Array(std::vector<std::shared_ptr<object::Object>> &&Elements)
    : Elements(std::move(Elements)) {}

ObjectType Array::type() const { return ObjectType::ARRAY_OBJ; }

std::string Array::inspect() const {
  std::stringstream SS;
  SS << "[";
  for (const auto &E : Elements) {
    SS << E->inspect();
    if (E.get() != Elements.back().get())
      SS << ", ";
  }

  SS << "]";
  return SS.str();
}

HashKey::HashKey(const std::shared_ptr<object::Object> &Key) : Key(Key) {}

bool HashKey::operator==(const HashKey &Other) const {
  return Key->equals(*Other.Key);
}

bool hasHashKey(const HashKey &Hash) {
  const auto ObjType = Hash.Key->type();
  return ObjType == ObjectType::BOOLEAN_OBJ ||
         ObjType == ObjectType::INTEGER_OBJ ||
         ObjType == ObjectType::STRING_OBJ;
}

size_t HashKeyHasher::operator()(const HashKey &Hash) const {
  const auto TypeHash =
      std::hash<uint64_t>()(static_cast<uint64_t>(Hash.Key->type()));
  return TypeHash ^ Hash.Key->hash();
}

Hash::Hash(std::unordered_map<HashKey, std::shared_ptr<object::Object>,
                              HashKeyHasher> &&Pairs)
    : Pairs(std::move(Pairs)) {}

ObjectType Hash::type() const { return ObjectType::HASH_OBJ; }

std::string Hash::inspect() const {
  std::stringstream SS;
  SS << "{";

  bool First = true;
  for (const auto &P : Pairs) {
    if (!First)
      SS << ", ";

    First = false;
    SS << P.first.Key->inspect() << ": " << P.second->inspect();
  }

  SS << "}";
  return SS.str();
}

ObjectType CompiledFunction::type() const {
  return ObjectType::COMPILED_FUNCTION_OBJ;
}

std::string CompiledFunction::inspect() const {
  std::stringstream SS;
  SS << "CompiledFunction[" << this << "]";
  return SS.str();
}

ObjectType Closure::type() const { return ObjectType::CLOSURE_OBJ; }

std::string Closure::inspect() const {
  std::stringstream SS;
  SS << "Closure[" << this << "]";
  return SS.str();
}

} // namespace monkey::object
