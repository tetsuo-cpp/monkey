#include "Compiler.h"

namespace monkey::compiler {

Compiler::Compiler(SymbolTable &SymTable,
                   std::vector<std::shared_ptr<object::Object>> &Constants)
    : Constants(Constants), SymTable(SymTable) {}

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

    if (lastInstructionIsPop())
      removeLastPop();

    // Emit an 'OpJump' with a bogus value.
    auto JumpPos = emit(code::OpCode::OpJump, {9999});

    int AfterConsequencePos = Instructions.Value.size();
    changeOperand(JumpNotTruthyPos, AfterConsequencePos);

    if (!IfE->Alternative) {
      emit(code::OpCode::OpNull, {});
    } else {
      compile(IfE->Alternative.get());

      if (lastInstructionIsPop())
        removeLastPop();
    }

    int AfterAlternativePos = Instructions.Value.size();
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
    const auto &Symbol = SymTable.define(Let->Name->Value);
    emit(code::OpCode::OpSetGlobal, {Symbol.Index});
    return;
  }

  const auto *Identifier = dynamic_cast<const ast::Identifier *>(Node);
  if (Identifier) {
    const auto *Symbol = SymTable.resolve(Identifier->Value);
    if (!Symbol)
      throw std::runtime_error("undefined variable " + Identifier->Value);

    emit(code::OpCode::OpGetGlobal, {Symbol->Index});
    return;
  }

  const auto *Bool = dynamic_cast<const ast::Boolean *>(Node);
  if (Bool) {
    if (Bool->Value)
      emit(code::OpCode::OpTrue, {});
    else
      emit(code::OpCode::OpFalse, {});
  }

  const auto *IntegerL = dynamic_cast<const ast::IntegerLiteral *>(Node);
  if (IntegerL) {
    auto Integer = std::make_shared<object::Integer>(IntegerL->Value);
    emit(code::OpCode::OpConstant, {addConstant(std::move(Integer))});
    return;
  }
}

ByteCode Compiler::byteCode() {
  return ByteCode(std::move(Instructions), Constants);
}

int Compiler::emit(code::OpCode Op, const std::vector<int> &Operands) {
  const auto &Ins = code::make(Op, Operands);
  auto Pos = addInstruction(Ins);

  setLastInstruction(Op, Pos);
  return Pos;
}

int Compiler::addInstruction(const std::vector<unsigned char> &Ins) {
  const auto PosNewInstruction = Instructions.Value.size();
  std::copy(Ins.begin(), Ins.end(), std::back_inserter(Instructions.Value));
  return PosNewInstruction;
}

void Compiler::setLastInstruction(code::OpCode Op, unsigned int Pos) {
  auto Previous = LastInstruction;
  EmittedInstruction Last(Op, Pos);
  PreviousInstruction = Previous;
  LastInstruction = Last;
}

bool Compiler::lastInstructionIsPop() const {
  return LastInstruction.Op == code::OpCode::OpPop;
}

void Compiler::removeLastPop() {
  Instructions.Value.erase(Instructions.Value.begin() +
                               LastInstruction.Position,
                           Instructions.Value.end());
  LastInstruction = PreviousInstruction;
}

void Compiler::replaceInstruction(unsigned int Pos,
                                  std::vector<unsigned char> &NewInstruction) {
  for (unsigned int Index = 0; Index < NewInstruction.size(); ++Index)
    Instructions.Value.at(Pos + Index) = NewInstruction.at(Index);
}

void Compiler::changeOperand(unsigned int OpPos, int Operand) {
  const auto Op = static_cast<code::OpCode>(Instructions.Value.at(OpPos));
  auto NewInstruction = code::make(Op, {Operand});

  replaceInstruction(OpPos, NewInstruction);
}

} // namespace monkey::compiler
