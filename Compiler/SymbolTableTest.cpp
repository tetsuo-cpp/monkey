#include "SymbolTable.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace monkey::compiler {

TEST(SymbolTableTests, testDefine) {
  std::unordered_map<std::string, Symbol> Expected = {
      {"a", {"a", GlobalScope, 0}}, {"b", {"b", GlobalScope, 1}}};

  SymbolTable Global;

  const auto &A = Global.define("a");
  ASSERT_EQ(A, Expected["a"]);

  const auto &B = Global.define("b");
  ASSERT_EQ(B, Expected["b"]);
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

} // namespace monkey::compiler
