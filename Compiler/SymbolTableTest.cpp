#include "SymbolTable.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::compiler {

TEST(SymbolTableTests, testDefine) {
  std::unordered_map<std::string, Symbol> Expected = {
      {"a", {"a", SymbolScope::GLOBAL_SCOPE, 0}},
      {"b", {"b", SymbolScope::GLOBAL_SCOPE, 1}},
      {"c", {"c", SymbolScope::LOCAL_SCOPE, 0}},
      {"d", {"d", SymbolScope::LOCAL_SCOPE, 1}},
      {"e", {"e", SymbolScope::LOCAL_SCOPE, 0}},
      {"f", {"f", SymbolScope::LOCAL_SCOPE, 1}}};

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

  const std::vector<Symbol> Expected = {{"a", SymbolScope::GLOBAL_SCOPE, 0},
                                        {"b", SymbolScope::GLOBAL_SCOPE, 1}};

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

  const std::vector<Symbol> Expected = {{"a", SymbolScope::GLOBAL_SCOPE, 0},
                                        {"b", SymbolScope::GLOBAL_SCOPE, 1},
                                        {"c", SymbolScope::LOCAL_SCOPE, 0},
                                        {"d", SymbolScope::LOCAL_SCOPE, 1}};

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
       {{"a", SymbolScope::GLOBAL_SCOPE, 0},
        {"b", SymbolScope::GLOBAL_SCOPE, 1},
        {"c", SymbolScope::LOCAL_SCOPE, 0},
        {"d", SymbolScope::LOCAL_SCOPE, 1}}},
      {&SecondLocal,
       {{"a", SymbolScope::GLOBAL_SCOPE, 0},
        {"b", SymbolScope::GLOBAL_SCOPE, 1},
        {"e", SymbolScope::LOCAL_SCOPE, 0},
        {"f", SymbolScope::LOCAL_SCOPE, 1}}}};

  for (auto &Test : Tests) {
    auto &ST = *Test.first;
    for (auto &Sym : Test.second) {
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
      {"a", SymbolScope::BUILTIN_SCOPE, 0},
      {"c", SymbolScope::BUILTIN_SCOPE, 1},
      {"e", SymbolScope::BUILTIN_SCOPE, 2},
      {"f", SymbolScope::BUILTIN_SCOPE, 3},
  };

  for (unsigned int I = 0; I < Expected.size(); ++I) {
    const auto &E = Expected.at(I);
    Global.defineBuiltIn(I, E.Name);
  }

  for (auto *Table : {&Global, &FirstLocal, &SecondLocal}) {
    for (const auto &Sym : Expected) {
      const auto *Result = Table->resolve(Sym.Name);
      ASSERT_THAT(Result, testing::NotNull());
      ASSERT_EQ(*Result, Sym);
    }
  }
}

TEST(SymbolTableTests, testResolveFree) {
  SymbolTable Global;
  Global.define("a");
  Global.define("b");

  SymbolTable FirstLocal(&Global);
  FirstLocal.define("c");
  FirstLocal.define("d");

  SymbolTable SecondLocal(&FirstLocal);
  SecondLocal.define("e");
  SecondLocal.define("f");

  const std::vector<
      std::tuple<SymbolTable *, std::vector<Symbol>, std::vector<Symbol>>>
      Tests = {{&FirstLocal,
                {{"a", SymbolScope::GLOBAL_SCOPE, 0},
                 {"b", SymbolScope::GLOBAL_SCOPE, 1},
                 {"c", SymbolScope::LOCAL_SCOPE, 0},
                 {"d", SymbolScope::LOCAL_SCOPE, 1}},
                {}},
               {&SecondLocal,
                {{"a", SymbolScope::GLOBAL_SCOPE, 0},
                 {"b", SymbolScope::GLOBAL_SCOPE, 1},
                 {"c", SymbolScope::FREE_SCOPE, 0},
                 {"d", SymbolScope::FREE_SCOPE, 1},
                 {"e", SymbolScope::LOCAL_SCOPE, 0},
                 {"f", SymbolScope::LOCAL_SCOPE, 1}},
                {{"c", SymbolScope::LOCAL_SCOPE, 0},
                 {"d", SymbolScope::LOCAL_SCOPE, 1}}}};

  for (auto &Test : Tests) {
    auto *Table = std::get<0>(Test);
    for (const auto &Sym : std::get<1>(Test)) {
      const auto *Result = Table->resolve(Sym.Name);
      ASSERT_THAT(Result, testing::NotNull());
      ASSERT_EQ(*Result, Sym);
    }

    const auto &ExpectedFreeSymbols = std::get<2>(Test);
    ASSERT_EQ(Table->FreeSymbols.size(), ExpectedFreeSymbols.size());
    ASSERT_EQ(Table->FreeSymbols, ExpectedFreeSymbols);
  }
}

TEST(SymbolTableTests, testResolveUnresolvableFree) {
  SymbolTable Global;
  Global.define("a");

  SymbolTable FirstLocal(&Global);
  FirstLocal.define("c");

  SymbolTable SecondLocal(&FirstLocal);
  SecondLocal.define("e");
  SecondLocal.define("f");

  const std::vector<Symbol> Expected = {{"a", SymbolScope::GLOBAL_SCOPE, 0},
                                        {"c", SymbolScope::FREE_SCOPE, 0},
                                        {"e", SymbolScope::LOCAL_SCOPE, 0},
                                        {"f", SymbolScope::LOCAL_SCOPE, 1}};

  for (const auto &Sym : Expected) {
    const auto *Result = SecondLocal.resolve(Sym.Name);
    ASSERT_THAT(Result, testing::NotNull());
    ASSERT_EQ(*Result, Sym);
  }

  const std::vector<std::string> NotExpected = {"b", "d"};

  for (const auto &Name : NotExpected)
    ASSERT_THAT(SecondLocal.resolve(Name), testing::IsNull());
}

} // namespace monkey::compiler
