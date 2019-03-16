#include "Environment.h"

namespace monkey::environment {

Environment::Environment() : Outer(nullptr) {}

Environment::Environment(Environment *Outer) : Outer(Outer) {}

std::shared_ptr<object::Object> Environment::get(const std::string &Name) {
  auto Iter = Store.find(Name);
  if (Iter != Store.end()) {
    return Iter->second;
  }

  if (Outer) {
    return Outer->get(Name);
  }

  return nullptr;
}

void Environment::set(const std::string &Name,
                      std::shared_ptr<object::Object> Value) {
  Store[Name] = std::move(Value);
}

} // namespace monkey::environment
