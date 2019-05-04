#include "SymbolTable.h"

namespace monkey::compiler {

bool Symbol::operator==(const Symbol &Other) const {
  return Name == Other.Name && Scope == Other.Scope && Index == Other.Index;
}

SymbolTable::SymbolTable() : NumDefinitions(0) {}

const Symbol &SymbolTable::define(const std::string &Name) {
  // TODO: Avoid double lookup.
  Store[Name] = Symbol{Name, GlobalScope, NumDefinitions};
  ++NumDefinitions;
  return Store[Name];
}

const Symbol *SymbolTable::resolve(const std::string &Name) const {
  const auto Iter = Store.find(Name);
  if (Iter == Store.end())
    return nullptr;

  return &Iter->second;
}

} // namespace monkey::compiler
