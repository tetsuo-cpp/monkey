#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace monkey::compiler {

using SymbolScope = std::string;

static SymbolScope GlobalScope("GLOBAL");
static SymbolScope LocalScope("LOCAL");
static SymbolScope BuiltInScope("BUILTIN");
static SymbolScope FreeScope("FREE");

struct Symbol {
  bool operator==(const Symbol &) const;

  std::string Name;
  SymbolScope Scope;
  int Index;
};

class SymbolTable {
public:
  SymbolTable();
  explicit SymbolTable(SymbolTable *);
  virtual ~SymbolTable() = default;

  const Symbol &define(const std::string &);
  const Symbol &defineBuiltIn(int, const std::string &);
  const Symbol &defineFree(const Symbol &);
  const Symbol *resolve(const std::string &);

  SymbolTable *Outer;
  int NumDefinitions;
  std::vector<Symbol> FreeSymbols;

private:
  std::unordered_map<std::string, Symbol> Store;
};

} // namespace monkey::compiler
