#include "Object.h"

namespace monkey::object {

const std::string INTEGER_OBJ("INTEGER");
const std::string BOOLEAN_OBJ("BOOLEAN");
const std::string NULL_OBJ("NULL");
const std::string RETURN_VALUE_OBJ("RETURN_VALUE");
const std::string ERROR_OBJ("ERROR");

Integer::Integer(int64_t Value) : Value(Value) {}

const ObjectType &Integer::type() const { return INTEGER_OBJ; }

std::string Integer::inspect() const { return std::to_string(Value); }

Boolean::Boolean(bool Value) : Value(Value) {}

const ObjectType &Boolean::type() const { return BOOLEAN_OBJ; }

std::string Boolean::inspect() const { return Value ? "true" : "false"; }

const ObjectType &Null::type() const { return NULL_OBJ; }

std::string Null::inspect() const { return "null"; }

ReturnValue::ReturnValue(std::unique_ptr<object::Object> Value)
    : Value(std::move(Value)) {}

const ObjectType &ReturnValue::type() const { return RETURN_VALUE_OBJ; }

std::string ReturnValue::inspect() const { return Value->inspect(); }

const ObjectType &Error::type() const { return ERROR_OBJ; }

std::string Error::inspect() const { return "ERROR: " + Message; }

} // namespace monkey::object
