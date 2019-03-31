#include "Object.h"

#include <sstream>

namespace monkey::object {

const std::string INTEGER_OBJ("INTEGER");
const std::string BOOLEAN_OBJ("BOOLEAN");
const std::string NULL_OBJ("NULL");
const std::string RETURN_VALUE_OBJ("RETURN_VALUE");
const std::string ERROR_OBJ("ERROR");
const std::string FUNCTION_OBJ("FUNCTION");
const std::string STRING_OBJ("STRING");
const std::string BUILTIN_OBJ("BUILTIN");
const std::string ARRAY_OBJ("ARRAY");

Integer::Integer(int64_t Value) : Value(Value) {}

const ObjectType &Integer::type() const { return INTEGER_OBJ; }

std::string Integer::inspect() const { return std::to_string(Value); }

Boolean::Boolean(bool Value) : Value(Value) {}

const ObjectType &Boolean::type() const { return BOOLEAN_OBJ; }

std::string Boolean::inspect() const { return Value ? "true" : "false"; }

const ObjectType &Null::type() const { return NULL_OBJ; }

std::string Null::inspect() const { return "null"; }

ReturnValue::ReturnValue(std::shared_ptr<object::Object> Value)
    : Value(std::move(Value)) {}

const ObjectType &ReturnValue::type() const { return RETURN_VALUE_OBJ; }

std::string ReturnValue::inspect() const { return Value->inspect(); }

const ObjectType &Error::type() const { return ERROR_OBJ; }

std::string Error::inspect() const { return "ERROR: " + Message; }

Function::Function(std::vector<std::unique_ptr<ast::Identifier>> &&Parameters,
                   std::unique_ptr<ast::BlockStatement> Body,
                   environment::Environment &Env)
    : Parameters(std::move(Parameters)), Body(std::move(Body)), Env(Env) {}

const ObjectType &Function::type() const { return FUNCTION_OBJ; }

std::string Function::inspect() const {
  std::stringstream SS;
  SS << "fn(";
  for (const auto &Param : Parameters) {
    SS << Param->string();
    if (&Param != &Parameters.back()) {
      SS << ", ";
    }
  }

  SS << ") {\n";
  SS << Body->string();
  SS << "\n}";
  return SS.str();
}

const ObjectType &String::type() const { return STRING_OBJ; }

std::string String::inspect() const { return Value; }

BuiltIn::BuiltIn(const BuiltInFunction &Fn) : Fn(Fn) {}

const ObjectType &BuiltIn::type() const { return BUILTIN_OBJ; }

std::string BuiltIn::inspect() const { return "builtin string"; }

Array::Array(std::vector<std::shared_ptr<object::Object>> &&Elements)
    : Elements(std::move(Elements)) {}

const ObjectType &Array::type() const { return ARRAY_OBJ; }

std::string Array::inspect() const {
  std::stringstream SS;
  SS << "[";
  for (const auto &E : Elements) {
    SS << E->inspect();
    if (E.get() != Elements.back().get()) {
      SS << ", ";
    }
  }

  SS << "]";
  return SS.str();
}

} // namespace monkey::object
