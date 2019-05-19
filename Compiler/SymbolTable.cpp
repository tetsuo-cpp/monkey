#include "SymbolTable.h"

namespace monkey::compiler {

bool Symbol::operator==(const Symbol &Other) const {
  return Name == Other.Name && Scope == Other.Scope && Index == Other.Index;
}

SymbolTable::SymbolTable() : Outer(nullptr), NumDefinitions(0) {}

SymbolTable::SymbolTable(const SymbolTable *Outer)
    : Outer(Outer), NumDefinitions(0) {}

const Symbol &SymbolTable::define(const std::string &Name) {
  if (Outer)
    Store[Name] = Symbol{Name, LocalScope, NumDefinitions};
  else
    Store[Name] = Symbol{Name, GlobalScope, NumDefinitions};

  // TODO: Avoid double lookup.
  ++NumDefinitions;
  return Store[Name];
}

const Symbol *SymbolTable::resolve(const std::string &Name) const {
  const auto Iter = Store.find(Name);
  if (Iter == Store.end()) {
    if (Outer)
      return Outer->resolve(Name);
    else
      return nullptr;
  }

  return &Iter->second;
}

} // namespace monkey::compiler
