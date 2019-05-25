#include "SymbolTable.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::compiler {

TEST(SymbolTableTests, testDefine) {
  std::unordered_map<std::string, Symbol> Expected = {
      {"a", {"a", GlobalScope, 0}}, {"b", {"b", GlobalScope, 1}},
      {"c", {"c", LocalScope, 0}},  {"d", {"d", LocalScope, 1}},
      {"e", {"e", LocalScope, 0}},  {"f", {"f", LocalScope, 1}}};

  SymbolTable Global;
  const auto &A = Global.define("a");
  ASSERT_EQ(A, Expected["a"]);

  const auto &B = Global.define("b");
  ASSERT_EQ(B, Expected["b"]);

  SymbolTable FirstLocal(&Global);
  const auto &C = FirstLocal.define("c");
  ASSERT_EQ(C, Expected["c"]);

  const auto &D = FirstLocal.define("d");
  ASSERT_EQ(D, Expected["d"]);

  SymbolTable SecondLocal(&FirstLocal);
  const auto &E = SecondLocal.define("e");
  ASSERT_EQ(E, Expected["e"]);

  const auto &F = SecondLocal.define("f");
  ASSERT_EQ(F, Expected["f"]);
}

TEST(SymbolTableTests, testResolveGlobal) {
  SymbolTable Global;
  Global.define("a");
  Global.define("b");

  const std::vector<Symbol> Expected = {{"a", GlobalScope, 0},
                                        {"b", GlobalScope, 1}};

  for (const auto &E : Expected) {
    const auto *Result = Global.resolve(E.Name);
    ASSERT_THAT(Result, testing::NotNull());
    ASSERT_EQ(*Result, E);
  }
}

TEST(SymbolTableTests, testResolveLocal) {
  SymbolTable Global;
  Global.define("a");
  Global.define("b");

  SymbolTable Local(&Global);
  Local.define("c");
  Local.define("d");

  const std::vector<Symbol> Expected = {{"a", GlobalScope, 0},
                                        {"b", GlobalScope, 1},
                                        {"c", LocalScope, 0},
                                        {"d", LocalScope, 1}};

  for (const auto &E : Expected) {
    const auto *Result = Local.resolve(E.Name);
    ASSERT_THAT(Result, testing::NotNull());
    ASSERT_EQ(*Result, E);
  }
}

TEST(SymbolTableTests, testResolveNestedLocal) {
  SymbolTable Global;
  Global.define("a");
  Global.define("b");

  SymbolTable FirstLocal(&Global);
  FirstLocal.define("c");
  FirstLocal.define("d");

  SymbolTable SecondLocal(&FirstLocal);
  SecondLocal.define("e");
  SecondLocal.define("f");

  const std::vector<std::pair<SymbolTable *, std::vector<Symbol>>> Tests = {
      {&FirstLocal,
       {{"a", GlobalScope, 0},
        {"b", GlobalScope, 1},
        {"c", LocalScope, 0},
        {"d", LocalScope, 1}}},
      {&SecondLocal,
       {{"a", GlobalScope, 0},
        {"b", GlobalScope, 1},
        {"e", LocalScope, 0},
        {"f", LocalScope, 1}}}};

  for (const auto &Test : Tests) {
    const auto &ST = *Test.first;
    for (const auto &Sym : Test.second) {
      const auto *Result = ST.resolve(Sym.Name);
      ASSERT_THAT(Result, testing::NotNull());
      ASSERT_EQ(*Result, Sym);
    }
  }
}

TEST(SymbolTableTests, testDefineResolveBuiltIns) {
  SymbolTable Global;
  SymbolTable FirstLocal(&Global);
  SymbolTable SecondLocal(&FirstLocal);

  const std::vector<Symbol> Expected = {
      {"a", BuiltInScope, 0},
      {"c", BuiltInScope, 1},
      {"e", BuiltInScope, 2},
      {"f", BuiltInScope, 3},
  };

  for (unsigned int I = 0; I < Expected.size(); ++I) {
    const auto &E = Expected.at(I);
    Global.defineBuiltIn(I, E.Name);
  }

  for (const auto &Table : {Global, FirstLocal, SecondLocal}) {
    for (const auto &Sym : Expected) {
      const auto *Result = Table.resolve(Sym.Name);
      ASSERT_THAT(Result, testing::NotNull());
      ASSERT_EQ(*Result, Sym);
    }
  }
}

} // namespace monkey::compiler
