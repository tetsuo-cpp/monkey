#include "SymbolTable.h"

namespace monkey::compiler {

const char *symbolScopeToString(SymbolScope Scope) {
  switch (Scope) {
  case SymbolScope::GLOBAL_SCOPE:
    return "GLOBAL_SCOPE";
  case SymbolScope::LOCAL_SCOPE:
    return "LOCAL_SCOPE";
  case SymbolScope::BUILTIN_SCOPE:
    return "BUILTIN_SCOPE";
  case SymbolScope::FREE_SCOPE:
    return "FREE_SCOPE";
  }

  return "UNKNOWN";
}

bool Symbol::operator==(const Symbol &Other) const {
  return Name == Other.Name && Scope == Other.Scope && Index == Other.Index;
}

SymbolTable::SymbolTable() : Outer(nullptr), NumDefinitions(0) {}

SymbolTable::SymbolTable(SymbolTable *Outer)
    : Outer(Outer), NumDefinitions(0) {}

const Symbol &SymbolTable::define(const std::string &Name) {
  Symbol S{Name, Outer ? SymbolScope::LOCAL_SCOPE : SymbolScope::GLOBAL_SCOPE,
           NumDefinitions};
  const auto &NewSym = (Store[Name] = std::move(S));

  ++NumDefinitions;
  return NewSym;
}

const Symbol &SymbolTable::defineBuiltIn(int Index, const std::string &Name) {
  Symbol S{Name, SymbolScope::BUILTIN_SCOPE, Index};
  const auto &NewSym = (Store[Name] = std::move(S));
  return NewSym;
}

const Symbol &SymbolTable::defineFree(const Symbol &Original) {
  FreeSymbols.push_back(Original);
  Symbol S{Original.Name, SymbolScope::FREE_SCOPE,
           static_cast<int>(FreeSymbols.size() - 1)};
  const auto &NewSym = (Store[Original.Name] = std::move(S));
  return NewSym;
}

const Symbol *SymbolTable::resolve(const std::string &Name) {
  const auto Iter = Store.find(Name);
  if (Iter == Store.end()) {
    if (Outer) {
      const auto *OuterSymbol = Outer->resolve(Name);
      if (!OuterSymbol || OuterSymbol->Scope == SymbolScope::GLOBAL_SCOPE ||
          OuterSymbol->Scope == SymbolScope::BUILTIN_SCOPE)
        return OuterSymbol;

      return &defineFree(*OuterSymbol);
    } else
      return nullptr;
  }

  return &Iter->second;
}

} // namespace monkey::compiler
