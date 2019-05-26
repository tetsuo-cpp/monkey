#include "Compiler.h"

#include <Object/BuiltIns.h>

namespace monkey::compiler {

Compiler::Compiler(SymbolTable &SymTable,
                   std::vector<std::shared_ptr<object::Object>> &Constants)
    : ScopeIndex(0), GlobalSymTable(SymTable), SymTable(&GlobalSymTable),
      Constants(Constants) {
  // Main scope.
  Scopes.emplace_back();

  for (unsigned int I = 0; I < object::BuiltIns.size(); ++I)
    GlobalSymTable.defineBuiltIn(I, object::BuiltIns.at(I).first);
}

void Compiler::compile(const ast::Node *Node) {
  const auto *Program = dynamic_cast<const ast::Program *>(Node);
  if (Program) {
    for (const auto &Statement : Program->Statements)
      compile(Statement.get());
    return;
  }

  const auto *ExprS = dynamic_cast<const ast::ExpressionStatement *>(Node);
  if (ExprS) {
    compile(ExprS->Expr.get());
    emit(code::OpCode::OpPop, {});
    return;
  }

  const auto *InfixExpr = dynamic_cast<const ast::InfixExpression *>(Node);
  if (InfixExpr) {
    if (InfixExpr->Operator == "<") {
      compile(InfixExpr->Right.get());
      compile(InfixExpr->Left.get());
      emit(code::OpCode::OpGreaterThan, {});
      return;
    }

    compile(InfixExpr->Left.get());
    compile(InfixExpr->Right.get());

    if (InfixExpr->Operator == "+")
      emit(code::OpCode::OpAdd, {});
    else if (InfixExpr->Operator == "-")
      emit(code::OpCode::OpSub, {});
    else if (InfixExpr->Operator == "*")
      emit(code::OpCode::OpMul, {});
    else if (InfixExpr->Operator == "/")
      emit(code::OpCode::OpDiv, {});
    else if (InfixExpr->Operator == ">")
      emit(code::OpCode::OpGreaterThan, {});
    else if (InfixExpr->Operator == "==")
      emit(code::OpCode::OpEqual, {});
    else if (InfixExpr->Operator == "!=")
      emit(code::OpCode::OpNotEqual, {});
    else
      throw std::runtime_error(
          std::string("unknown operator " + InfixExpr->Operator));

    return;
  }

  const auto *PrefixE = dynamic_cast<const ast::PrefixExpression *>(Node);
  if (PrefixE) {
    compile(PrefixE->Right.get());

    if (PrefixE->Operator == "!")
      emit(code::OpCode::OpBang, {});
    else if (PrefixE->Operator == "-")
      emit(code::OpCode::OpMinus, {});
    else
      throw std::runtime_error("unknown operator " + PrefixE->Operator);
    return;
  }

  const auto *IfE = dynamic_cast<const ast::IfExpression *>(Node);
  if (IfE) {
    compile(IfE->Condition.get());

    // Emit an 'OpJumpNotTruthy' with a bogus value.
    auto JumpNotTruthyPos = emit(code::OpCode::OpJumpNotTruthy, {9999});

    compile(IfE->Consequence.get());

    if (lastInstructionIs(code::OpCode::OpPop))
      removeLastPop();

    // Emit an 'OpJump' with a bogus value.
    auto JumpPos = emit(code::OpCode::OpJump, {9999});

    int AfterConsequencePos = currentInstructions().Value.size();
    changeOperand(JumpNotTruthyPos, AfterConsequencePos);

    if (!IfE->Alternative) {
      emit(code::OpCode::OpNull, {});
    } else {
      compile(IfE->Alternative.get());

      if (lastInstructionIs(code::OpCode::OpPop))
        removeLastPop();
    }

    int AfterAlternativePos = currentInstructions().Value.size();
    changeOperand(JumpPos, AfterAlternativePos);
    return;
  }

  const auto *Block = dynamic_cast<const ast::BlockStatement *>(Node);
  if (Block) {
    for (const auto &Statement : Block->Statements)
      compile(Statement.get());

    return;
  }

  const auto *Let = dynamic_cast<const ast::LetStatement *>(Node);
  if (Let) {
    compile(Let->Value.get());
    const auto &Symbol = SymTable->define(Let->Name->Value);
    if (Symbol.Scope == GlobalScope)
      emit(code::OpCode::OpSetGlobal, {Symbol.Index});
    else
      emit(code::OpCode::OpSetLocal, {Symbol.Index});
    return;
  }

  const auto *Identifier = dynamic_cast<const ast::Identifier *>(Node);
  if (Identifier) {
    const auto *Symbol = SymTable->resolve(Identifier->Value);
    if (!Symbol)
      throw std::runtime_error("undefined variable " + Identifier->Value);

    loadSymbol(*Symbol);
    return;
  }

  const auto *Bool = dynamic_cast<const ast::Boolean *>(Node);
  if (Bool) {
    if (Bool->Value)
      emit(code::OpCode::OpTrue, {});
    else
      emit(code::OpCode::OpFalse, {});
    return;
  }

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL) {
    auto Integer = std::make_shared<object::Integer>(IntegerL->Value);
    emit(code::OpCode::OpConstant, {addConstant(std::move(Integer))});
    return;
  }

  const auto *StringL = dynamic_cast<const ast::String *>(Node);
  if (StringL) {
    auto String = std::make_shared<object::String>(StringL->Value);
    emit(code::OpCode::OpConstant, {addConstant(std::move(String))});
    return;
  }

  const auto *ArrayL = dynamic_cast<const ast::ArrayLiteral *>(Node);
  if (ArrayL) {
    for (const auto &Elem : ArrayL->Elements)
      compile(Elem.get());

    emit(code::OpCode::OpArray, {static_cast<int>(ArrayL->Elements.size())});
    return;
  }

  const auto *HashL = dynamic_cast<const ast::HashLiteral *>(Node);
  if (HashL) {
    std::vector<std::pair<ast::Expression *, ast::Expression *>> Keys;
    for (const auto &K : HashL->Pairs)
      Keys.emplace_back(K.first.get(), K.second.get());

    std::sort(Keys.begin(), Keys.end(),
              [](const std::pair<ast::Expression *, ast::Expression *> &L,
                 const std::pair<ast::Expression *, ast::Expression *> &R) {
                return L.first->string() < R.first->string();
              });

    for (const auto &K : Keys) {
      compile(K.first);
      compile(K.second);
    }

    emit(code::OpCode::OpHash, {static_cast<int>(HashL->Pairs.size() * 2)});
    return;
  }

  const auto *Index = dynamic_cast<const ast::IndexExpression *>(Node);
  if (Index) {
    compile(Index->Left.get());
    compile(Index->Index.get());
    emit(code::OpCode::OpIndex, {});
    return;
  }

  const auto *FunctionL = dynamic_cast<const ast::FunctionLiteral *>(Node);
  if (FunctionL) {
    enterScope();
    for (const auto &P : FunctionL->Parameters)
      SymTable->define(P->Value);

    compile(FunctionL->Body.get());

    if (lastInstructionIs(code::OpCode::OpPop))
      replaceLastPopWithReturn();
    if (!lastInstructionIs(code::OpCode::OpReturnValue))
      emit(code::OpCode::OpReturn, {});

    const auto NumLocals = SymTable->NumDefinitions;
    auto Ins = leaveScope();
    auto CompiledFn = std::make_unique<object::CompiledFunction>(
        std::move(Ins), NumLocals, FunctionL->Parameters.size());
    const auto FnIndex = addConstant(std::move(CompiledFn));
    emit(code::OpCode::OpClosure, {FnIndex, 0});
    return;
  }

  const auto *Return = dynamic_cast<const ast::ReturnStatement *>(Node);
  if (Return) {
    compile(Return->ReturnValue.get());
    emit(code::OpCode::OpReturnValue, {});
    return;
  }

  const auto *Call = dynamic_cast<const ast::CallExpression *>(Node);
  if (Call) {
    compile(Call->Function.get());
    for (const auto &A : Call->Arguments)
      compile(A.get());

    emit(code::OpCode::OpCall, {static_cast<int>(Call->Arguments.size())});
    return;
  }
}

ByteCode Compiler::byteCode() {
  return ByteCode(std::move(currentInstructions()), Constants);
}

int Compiler::emit(code::OpCode Op, const std::vector<int> &Operands) {
  const auto &Ins = code::make(Op, Operands);
  auto Pos = addInstruction(Ins);

  setLastInstruction(Op, Pos);
  return Pos;
}

int Compiler::addInstruction(const std::vector<unsigned char> &Ins) {
  auto &CurrentIns = currentInstructions();
  const auto PosNewInstruction = CurrentIns.Value.size();
  std::copy(Ins.begin(), Ins.end(), std::back_inserter(CurrentIns.Value));
  return PosNewInstruction;
}

void Compiler::setLastInstruction(code::OpCode Op, unsigned int Pos) {
  auto &CurrentScope = Scopes.at(ScopeIndex);
  auto Previous = CurrentScope.LastInstruction;
  EmittedInstruction Last(Op, Pos);
  CurrentScope.PreviousInstruction = Previous;
  CurrentScope.LastInstruction = Last;
}

bool Compiler::lastInstructionIs(code::OpCode Op) const {
  if (currentInstructions().Value.empty())
    return false;

  return Scopes.at(ScopeIndex).LastInstruction.Op == Op;
}

void Compiler::removeLastPop() {
  auto &CurrentScope = Scopes.at(ScopeIndex);
  CurrentScope.Instructions.Value.erase(
      CurrentScope.Instructions.Value.begin() +
          CurrentScope.LastInstruction.Position,
      CurrentScope.Instructions.Value.end());
  CurrentScope.LastInstruction = CurrentScope.PreviousInstruction;
}

void Compiler::replaceInstruction(unsigned int Pos,
                                  std::vector<unsigned char> &NewInstruction) {
  auto &CurrentIns = currentInstructions();
  for (unsigned int Index = 0; Index < NewInstruction.size(); ++Index)
    CurrentIns.Value.at(Pos + Index) = NewInstruction.at(Index);
}

void Compiler::changeOperand(unsigned int OpPos, int Operand) {
  const auto Op =
      static_cast<code::OpCode>(currentInstructions().Value.at(OpPos));
  auto NewInstruction = code::make(Op, {Operand});

  replaceInstruction(OpPos, NewInstruction);
}

code::Instructions &Compiler::currentInstructions() {
  return const_cast<code::Instructions &>(
      const_cast<const Compiler *>(this)->currentInstructions());
}

const code::Instructions &Compiler::currentInstructions() const {
  return Scopes.at(ScopeIndex).Instructions;
}

void Compiler::enterScope() {
  Scopes.emplace_back();
  ++ScopeIndex;
  SymTables.push_back(std::make_unique<SymbolTable>(SymTable));
  SymTable = SymTables.back().get();
}

code::Instructions Compiler::leaveScope() {
  SymTables.pop_back();
  if (SymTables.empty())
    SymTable = &GlobalSymTable;
  else
    SymTable = SymTables.back().get();

  auto Ins = currentInstructions();
  Scopes.pop_back();
  --ScopeIndex;
  return Ins;
}

void Compiler::replaceLastPopWithReturn() {
  auto &CurrentScope = Scopes.at(ScopeIndex);
  const auto LastPos = CurrentScope.LastInstruction.Position;

  auto ReturnIns = code::make(code::OpCode::OpReturnValue, {});
  replaceInstruction(LastPos, ReturnIns);

  CurrentScope.LastInstruction.Op = code::OpCode::OpReturnValue;
}

void Compiler::loadSymbol(const Symbol &S) {
  if (S.Scope == GlobalScope)
    emit(code::OpCode::OpGetGlobal, {S.Index});
  else if (S.Scope == LocalScope)
    emit(code::OpCode::OpGetLocal, {S.Index});
  else if (S.Scope == BuiltInScope)
    emit(code::OpCode::OpGetBuiltIn, {S.Index});
}

} // namespace monkey::compiler
