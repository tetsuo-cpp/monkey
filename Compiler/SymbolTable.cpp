#include "SymbolTable.h"

namespace monkey::compiler {

bool Symbol::operator==(const Symbol &Other) const {
  return Name == Other.Name && Scope == Other.Scope && Index == Other.Index;
}

SymbolTable::SymbolTable() : Outer(nullptr), NumDefinitions(0) {}

SymbolTable::SymbolTable(SymbolTable *Outer)
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

const Symbol &SymbolTable::defineBuiltIn(int Index, const std::string &Name) {
  Symbol S{Name, BuiltInScope, Index};
  Store[Name] = std::move(S);
  return Store[Name];
}

const Symbol &SymbolTable::defineFree(const Symbol &Original) {
  FreeSymbols.push_back(Original);
  Store[Original.Name] = Symbol{Original.Name, FreeScope,
                                static_cast<int>(FreeSymbols.size() - 1)};
  return Store[Original.Name];
}

const Symbol *SymbolTable::resolve(const std::string &Name) {
  const auto Iter = Store.find(Name);
  if (Iter == Store.end()) {
    if (Outer) {
      const auto *OuterSymbol = Outer->resolve(Name);
      if (!OuterSymbol || OuterSymbol->Scope == GlobalScope ||
          OuterSymbol->Scope == BuiltInScope)
        return OuterSymbol;

      return &defineFree(*OuterSymbol);
    } else
      return nullptr;
  }

  return &Iter->second;
}

} // namespace monkey::compiler
