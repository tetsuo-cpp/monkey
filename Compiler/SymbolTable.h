#pragma once

#include <string>
#include <unordered_map>

namespace monkey::compiler {

using SymbolScope = std::string;

static SymbolScope GlobalScope("GLOBAL");
static SymbolScope LocalScope("LOCAL");

struct Symbol {
  bool operator==(const Symbol &) const;

  std::string Name;
  SymbolScope Scope;
  int Index;
};

class SymbolTable {
public:
  SymbolTable();
  explicit SymbolTable(const SymbolTable *);
  virtual ~SymbolTable() = default;

  const Symbol &define(const std::string &);
  const Symbol *resolve(const std::string &) const;

  const SymbolTable *Outer;
  int NumDefinitions;

private:
  std::unordered_map<std::string, Symbol> Store;
};

} // namespace monkey::compiler
