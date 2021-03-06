#include "Compiler.h"

#include <Lexer/Lexer.h>
#include <Parser/Parser.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <variant>

namespace monkey::compiler::test {

std::unique_ptr<ast::Program> parse(const std::string &Input) {
  lexer::Lexer L(Input);
  parser::Parser P(L);
  return P.parseProgram();
}

class TestCompiler : public Compiler {
public:
  TestCompiler(SymbolTable &ST,
               std::vector<std::shared_ptr<object::Object>> &Constants)
      : Compiler(ST, Constants) {}
  virtual ~TestCompiler() = default;

  // Expose some protected functions for testing.
  int emit(code::OpCode Op, const std::vector<int> &Operands) {
    return Compiler::emit(Op, Operands);
  }
  void enterScope() { Compiler::enterScope(); }
  code::Instructions leaveScope() { return Compiler::leaveScope(); }

  // Expose some protected data for testing.
  const std::vector<CompilationScope> &getScopes() const { return Scopes; }
  int getScopeIndex() const { return ScopeIndex; }
  const SymbolTable &getGlobalSymbolTable() const { return GlobalSymTable; }
  const SymbolTable *getSymbolTable() const { return SymTable; }
  const std::vector<std::unique_ptr<SymbolTable>> &getSymbolTables() const {
    return SymTables;
  }
};

using ConstantType =
    std::variant<int, std::string, std::vector<code::Instructions>>;

struct CompilerTestCase {
  const std::string Input;
  const std::vector<ConstantType> ExpectedConstants;
  const std::vector<code::Instructions> ExpectedInstructions;
};

code::Instructions
concatInstructions(const std::vector<code::Instructions> &Instructions) {
  code::Instructions Out;
  for (const auto &Ins : Instructions)
    std::copy(Ins.Value.begin(), Ins.Value.end(),
              std::back_inserter(Out.Value));

  return Out;
}

void testIntegerObject(int64_t Expected, const object::Object *Actual) {
  const auto *Integer = dynamic_cast<const object::Integer *>(Actual);
  ASSERT_THAT(Integer, testing::NotNull());
  ASSERT_EQ(Integer->Value, Expected);
}

void testStringObject(const std::string &Expected,
                      const object::Object *Actual) {
  const auto *String = dynamic_cast<const object::String *>(Actual);
  ASSERT_THAT(String, testing::NotNull());
  ASSERT_EQ(String->Value, Expected);
}

void testInstructions(const std::vector<code::Instructions> &Expected,
                      const code::Instructions &Actual) {
  auto Concatted = concatInstructions(Expected);
  ASSERT_EQ(Actual.Value.size(), Concatted.Value.size());
  for (unsigned int I = 0; I < Actual.Value.size(); ++I)
    ASSERT_EQ(Actual.Value.at(I), Concatted.Value.at(I));
}

template <typename... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts> Overloaded(Ts...)->Overloaded<Ts...>;

void testConstants(const std::vector<ConstantType> &Expected,
                   const std::vector<std::shared_ptr<object::Object>> &Actual) {
  ASSERT_EQ(Expected.size(), Actual.size());
  for (unsigned int I = 0; I < Expected.size(); ++I) {
    std::visit(
        Overloaded{[&Actual, I](const int Arg) {
                     testIntegerObject(Arg, Actual.at(I).get());
                   },
                   [&Actual, I](const std::string &Arg) {
                     testStringObject(Arg, Actual.at(I).get());
                   },
                   [&Actual, I](const std::vector<code::Instructions> &Arg) {
                     const auto *Fn =
                         dynamic_cast<const object::CompiledFunction *>(
                             Actual.at(I).get());
                     ASSERT_THAT(Fn, testing::NotNull());
                     testInstructions(Arg, Fn->Ins);
                   }},
        Expected.at(I));
  }
}

void runCompilerTests(const std::vector<CompilerTestCase> &Tests) {
  for (const auto &Test : Tests) {
    const auto Program = parse(Test.Input);

    SymbolTable ST;
    std::vector<std::shared_ptr<object::Object>> Constants;
    TestCompiler C(ST, Constants);
    ASSERT_NO_THROW(C.compile(Program.get()));

    const auto ByteCode = C.byteCode();
    testInstructions(Test.ExpectedInstructions, ByteCode.Instructions);
    testConstants(Test.ExpectedConstants, ByteCode.Constants);
  }
}

TEST(CompilerTests, testIntegerArithmetic) {
  const std::vector<CompilerTestCase> Tests = {
      {"1 + 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1; 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpPop, {}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 - 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpSub, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 * 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpMul, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"2 / 1",
       {2, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpDiv, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"-1",
       {1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpMinus, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testBooleanExpressions) {
  const std::vector<CompilerTestCase> Tests = {
      {"true",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"false",
       {},
       {code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 > 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpGreaterThan, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 < 2",
       {2, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpGreaterThan, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 == 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"1 != 2",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpNotEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"true == false",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"true != false",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpFalse, {}),
        code::make(code::OpCode::OpNotEqual, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"!true",
       {},
       {code::make(code::OpCode::OpTrue, {}),
        code::make(code::OpCode::OpBang, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testConditionals) {
  const std::vector<CompilerTestCase> Tests = {
      {"if (true) { 10 }; 3333;",
       {10, 3333},
       {// 0000
        code::make(code::OpCode::OpTrue, {}),
        // 0001
        code::make(code::OpCode::OpJumpNotTruthy, {10}),
        // 0004
        code::make(code::OpCode::OpConstant, {0}),
        // 0007
        code::make(code::OpCode::OpJump, {11}),
        // 0010
        code::make(code::OpCode::OpNull, {}),
        // 0011
        code::make(code::OpCode::OpPop, {}),
        // 0012
        code::make(code::OpCode::OpConstant, {1}),
        // 0015
        code::make(code::OpCode::OpPop, {})}},
      {"if (true) { 10 } else { 20 }; 3333;",
       {10, 20, 3333},
       {// 0000
        code::make(code::OpCode::OpTrue, {}),
        // 0001
        code::make(code::OpCode::OpJumpNotTruthy, {10}),
        // 0004
        code::make(code::OpCode::OpConstant, {0}),
        // 0007
        code::make(code::OpCode::OpJump, {13}),
        // 0010
        code::make(code::OpCode::OpConstant, {1}),
        // 0013
        code::make(code::OpCode::OpPop, {}),
        // 0014
        code::make(code::OpCode::OpConstant, {2}),
        // 0017
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testLetStatements) {
  const std::vector<CompilerTestCase> Tests = {
      {"let one = 1;"
       "let two = 2;",
       {1, 2},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpSetGlobal, {1})}},
      {"let one = 1;"
       "one;",
       {1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpGetGlobal, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"let one = 1;"
       "let two = one;"
       "two;",
       {1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpGetGlobal, {0}),
        code::make(code::OpCode::OpSetGlobal, {1}),
        code::make(code::OpCode::OpGetGlobal, {1}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testStringExpressions) {
  const std::vector<CompilerTestCase> Tests = {
      {"\"monkey\"",
       {"monkey"},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"\"mon\" + \"key\"",
       {"mon", "key"},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testArrayLiterals) {
  const std::vector<CompilerTestCase> Tests = {
      {"[]",
       {},
       {code::make(code::OpCode::OpArray, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"[1, 2, 3]",
       {1, 2, 3},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpArray, {3}),
        code::make(code::OpCode::OpPop, {})}},
      {"[1 + 2, 3 - 4, 5 * 6]",
       {1, 2, 3, 4, 5, 6},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpSub, {}),
        code::make(code::OpCode::OpConstant, {4}),
        code::make(code::OpCode::OpConstant, {5}),
        code::make(code::OpCode::OpMul, {}),
        code::make(code::OpCode::OpArray, {3}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testHashLiterals) {
  const std::vector<CompilerTestCase> Tests = {
      {"{}",
       {},
       {code::make(code::OpCode::OpHash, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"{1: 2, 3: 4, 5: 6}",
       {1, 2, 3, 4, 5, 6},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpConstant, {4}),
        code::make(code::OpCode::OpConstant, {5}),
        code::make(code::OpCode::OpHash, {6}),
        code::make(code::OpCode::OpPop, {})}},
      {"{1: 2 + 3, 4: 5 * 6}",
       {1, 2, 3, 4, 5, 6},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpConstant, {4}),
        code::make(code::OpCode::OpConstant, {5}),
        code::make(code::OpCode::OpMul, {}),
        code::make(code::OpCode::OpHash, {4}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testIndexExpressions) {
  const std::vector<CompilerTestCase> Tests = {
      {"[1, 2, 3][1 + 1]",
       {1, 2, 3, 1, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpArray, {3}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpConstant, {4}),
        code::make(code::OpCode::OpAdd, {}),
        code::make(code::OpCode::OpIndex, {}),
        code::make(code::OpCode::OpPop, {})}},
      {"{1: 2}[2 - 1]",
       {1, 2, 2, 1},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpHash, {2}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpSub, {}),
        code::make(code::OpCode::OpIndex, {}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testFunctions) {
  const std::vector<CompilerTestCase> Tests = {
      {"fn() { return 5 + 10; }",
       {ConstantType(5), ConstantType(10),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {0}),
            code::make(code::OpCode::OpConstant, {1}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {2, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn() { 5 + 10 }",
       {ConstantType(5), ConstantType(10),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {0}),
            code::make(code::OpCode::OpConstant, {1}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {2, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn() { 1; 2 }",
       {ConstantType(1), ConstantType(2),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {0}),
            code::make(code::OpCode::OpPop, {}),
            code::make(code::OpCode::OpConstant, {1}),
            code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {2, 0}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testCompilerScopes) {
  SymbolTable ST;
  std::vector<std::shared_ptr<object::Object>> Constants;
  TestCompiler C(ST, Constants);
  ASSERT_EQ(C.getScopeIndex(), 0);

  const auto *GlobalSymbolTable = C.getSymbolTable();

  C.emit(code::OpCode::OpMul, {});

  C.enterScope();
  ASSERT_EQ(C.getScopeIndex(), 1);

  C.emit(code::OpCode::OpSub, {});

  ASSERT_EQ(C.getScopes().at(C.getScopeIndex()).Instructions.Value.size(), 1);

  auto Last = C.getScopes().at(C.getScopeIndex()).LastInstruction;
  ASSERT_EQ(Last.Op, code::OpCode::OpSub);

  EXPECT_EQ(C.getSymbolTable()->Outer, GlobalSymbolTable);

  C.leaveScope();
  ASSERT_EQ(C.getScopeIndex(), 0);

  EXPECT_EQ(C.getSymbolTable(), GlobalSymbolTable);
  EXPECT_THAT(C.getSymbolTable()->Outer, testing::IsNull());

  C.emit(code::OpCode::OpAdd, {});
  ASSERT_EQ(C.getScopes().at(C.getScopeIndex()).Instructions.Value.size(), 2);

  Last = C.getScopes().at(C.getScopeIndex()).LastInstruction;
  ASSERT_EQ(Last.Op, code::OpCode::OpAdd);

  auto Prev = C.getScopes().at(C.getScopeIndex()).PreviousInstruction;
  ASSERT_EQ(Prev.Op, code::OpCode::OpMul);
}

TEST(CompilerTests, testFunctionsWithoutReturnValue) {
  const std::vector<CompilerTestCase> Tests = {
      {"fn() {}",
       {ConstantType(std::vector<code::Instructions>{
           code::make(code::OpCode::OpReturn, {})})},
       {code::make(code::OpCode::OpClosure, {0, 0}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testFunctionCalls) {
  const std::vector<CompilerTestCase> Tests = {
      {"fn() { 24 }()",
       {ConstantType(24), ConstantType(std::vector<code::Instructions>{
                              code::make(code::OpCode::OpConstant, {0}),
                              code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {1, 0}),
        code::make(code::OpCode::OpCall, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"let noArg = fn() { 24 };"
       "noArg();",
       {ConstantType(24), ConstantType(std::vector<code::Instructions>{
                              code::make(code::OpCode::OpConstant, {0}),
                              code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {1, 0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpGetGlobal, {0}),
        code::make(code::OpCode::OpCall, {0}),
        code::make(code::OpCode::OpPop, {})}},
      {"let oneArg = fn(a) { a };"
       "oneArg(24);",
       {ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpReturnValue, {})}),
        ConstantType(24)},
       {code::make(code::OpCode::OpClosure, {0, 0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpGetGlobal, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpCall, {1}),
        code::make(code::OpCode::OpPop, {})}},
      {"let manyArg = fn(a, b, c) { a; b; c };"
       "manyArg(24, 25, 26);",
       {ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpPop, {}),
            code::make(code::OpCode::OpGetLocal, {1}),
            code::make(code::OpCode::OpPop, {}),
            code::make(code::OpCode::OpGetLocal, {2}),
            code::make(code::OpCode::OpReturnValue, {})}),
        ConstantType(24), ConstantType(25), ConstantType(26)},
       {code::make(code::OpCode::OpClosure, {0, 0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpGetGlobal, {0}),
        code::make(code::OpCode::OpConstant, {1}),
        code::make(code::OpCode::OpConstant, {2}),
        code::make(code::OpCode::OpConstant, {3}),
        code::make(code::OpCode::OpCall, {3}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testLetStatementScopes) {
  const std::vector<CompilerTestCase> Tests = {
      {"let num = 55;"
       "fn() { num }",
       {ConstantType(55), ConstantType(std::vector<code::Instructions>{
                              code::make(code::OpCode::OpGetGlobal, {0}),
                              code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpClosure, {1, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn() {"
       "let num = 55;"
       "num"
       "}",
       {ConstantType(55), ConstantType(std::vector<code::Instructions>{
                              code::make(code::OpCode::OpConstant, {0}),
                              code::make(code::OpCode::OpSetLocal, {0}),
                              code::make(code::OpCode::OpGetLocal, {0}),
                              code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {1, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn() {"
       "let a = 55;"
       "let b = 77;"
       "a + b"
       "}",
       {ConstantType(55), ConstantType(77),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {0}),
            code::make(code::OpCode::OpSetLocal, {0}),
            code::make(code::OpCode::OpConstant, {1}),
            code::make(code::OpCode::OpSetLocal, {1}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpGetLocal, {1}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {2, 0}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testBuiltIns) {
  const std::vector<CompilerTestCase> Tests = {
      {"len([]);"
       "push([], 1);",
       {1},
       {code::make(code::OpCode::OpGetBuiltIn, {0}),
        code::make(code::OpCode::OpArray, {0}),
        code::make(code::OpCode::OpCall, {1}),
        code::make(code::OpCode::OpPop, {}),
        code::make(code::OpCode::OpGetBuiltIn, {4}),
        code::make(code::OpCode::OpArray, {0}),
        code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpCall, {2}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn() { len([]) }",
       {ConstantType(std::vector<code::Instructions>{
           code::make(code::OpCode::OpGetBuiltIn, {0}),
           code::make(code::OpCode::OpArray, {0}),
           code::make(code::OpCode::OpCall, {1}),
           code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpClosure, {0, 0}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

TEST(CompilerTests, testClosures) {
  const std::vector<CompilerTestCase> Tests = {
      {"fn(a) {"
       "fn(b) {"
       "a + b"
       "}"
       "}",
       {std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetFree, {0}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})},
        std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpClosure, {0, 1}),
            code::make(code::OpCode::OpReturnValue, {})}},
       {code::make(code::OpCode::OpClosure, {1, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"fn(a) {"
       "fn(b) {"
       "fn(c) {"
       "a + b + c"
       "}"
       "}"
       "}",
       {std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetFree, {0}),
            code::make(code::OpCode::OpGetFree, {1}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})},
        std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetFree, {0}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpClosure, {0, 2}),
            code::make(code::OpCode::OpReturnValue, {})},
        std::vector<code::Instructions>{
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpClosure, {1, 1}),
            code::make(code::OpCode::OpReturnValue, {})}},
       {code::make(code::OpCode::OpClosure, {2, 0}),
        code::make(code::OpCode::OpPop, {})}},
      {"let global = 55;"
       "fn() {"
       "let a = 66;"
       "fn() {"
       "let b = 77;"
       "fn() {"
       "let c = 88;"
       "global + a + b + c;"
       "}"
       "}"
       "}",
       {ConstantType(55), ConstantType(66), ConstantType(77), ConstantType(88),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {3}),
            code::make(code::OpCode::OpSetLocal, {0}),
            code::make(code::OpCode::OpGetGlobal, {0}),
            code::make(code::OpCode::OpGetFree, {0}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpGetFree, {1}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpAdd, {}),
            code::make(code::OpCode::OpReturnValue, {})}),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {2}),
            code::make(code::OpCode::OpSetLocal, {0}),
            code::make(code::OpCode::OpGetFree, {0}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpClosure, {4, 2}),
            code::make(code::OpCode::OpReturnValue, {})}),
        ConstantType(std::vector<code::Instructions>{
            code::make(code::OpCode::OpConstant, {1}),
            code::make(code::OpCode::OpSetLocal, {0}),
            code::make(code::OpCode::OpGetLocal, {0}),
            code::make(code::OpCode::OpClosure, {5, 1}),
            code::make(code::OpCode::OpReturnValue, {})})},
       {code::make(code::OpCode::OpConstant, {0}),
        code::make(code::OpCode::OpSetGlobal, {0}),
        code::make(code::OpCode::OpClosure, {6, 0}),
        code::make(code::OpCode::OpPop, {})}}};

  runCompilerTests(Tests);
}

} // namespace monkey::compiler::test
