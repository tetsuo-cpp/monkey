#include "Object.h"

namespace monkey::object {

namespace {

const std::string INTEGER_OBJ("INTEGER");
const std::string BOOLEAN_OBJ("BOOLEAN");
const std::string NULL_OBJ("NULL");

} // namespace

Integer::Integer(int64_t Value) : Value(Value) {}

const ObjectType &Integer::type() const { return INTEGER_OBJ; }

std::string Integer::inspect() const { return std::to_string(Value); }

Boolean::Boolean(bool Value) : Value(Value) {}

const ObjectType &Boolean::type() const { return BOOLEAN_OBJ; }

std::string Boolean::inspect() const { return Value ? "true" : "false"; }

const ObjectType &Null::type() const { return NULL_OBJ; }

std::string Null::inspect() const { return "null"; }

} // namespace monkey::object
