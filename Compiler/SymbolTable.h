#pragma once

#include <string>
#include <unordered_map>

namespace monkey::compiler {

using SymbolScope = std::string;

static SymbolScope GlobalScope("GLOBAL");

struct Symbol {
  bool operator==(const Symbol &) const;

  std::string Name;
  SymbolScope Scope;
  int Index;
};

class SymbolTable {
public:
  SymbolTable();
  virtual ~SymbolTable() = default;

  const Symbol &define(const std::string &);
  const Symbol *resolve(const std::string &) const;

private:
  std::unordered_map<std::string, Symbol> Store;
  int NumDefinitions;
};

} // namespace monkey::compiler
