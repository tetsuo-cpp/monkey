#include "Compiler.h"

namespace monkey::compiler {

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

ByteCode Compiler::byteCode() const {
  return ByteCode(std::move(Instructions), std::move(Constants));
}

int Compiler::emit(code::OpCode Op, const std::vector<int> &Operands) {
  const auto &Ins = code::make(Op, Operands);
  auto Pos = addInstruction(Ins);
  return Pos;
}

int Compiler::addInstruction(const std::vector<unsigned char> &Ins) {
  const auto PosNewInstruction = Instructions.Value.size();
  std::copy(Ins.begin(), Ins.end(), std::back_inserter(Instructions.Value));
  return PosNewInstruction;
}

} // namespace monkey::compiler
