#include <Compiler/Compiler.h>
#include <Evaluator/Evaluator.h>
#include <Lexer/Lexer.h>
#include <Parser/Parser.h>
#include <VM/VM.h>

#include <iostream>

static const std::string Input("let fibonacci = fn(x) {"
                               "if (x == 0) {"
                               "0"
                               "} else {"
                               "if (x == 1) {"
                               "return 1;"
                               "} else {"
                               "fibonacci(x - 1) + fibonacci(x - 2);"
                               "}"
                               "}"
                               "};"
                               "let x = fibonacci(35);");

int main(int argc, char **argv) {
  if (argc != 2)
    return EXIT_FAILURE;

  const std::string Engine(argv[1]);

  monkey::lexer::Lexer L(Input);
  monkey::parser::Parser P(L);

  auto Program = P.parseProgram();

  std::shared_ptr<monkey::object::Object> ResultPtr;
  const monkey::object::Object *Result;
  if (Engine == "vm") {
    monkey::compiler::SymbolTable ST;
    std::vector<std::shared_ptr<monkey::object::Object>> Constants;
    std::array<std::shared_ptr<monkey::object::Object>, GlobalsSize> Globals;
    monkey::compiler::Compiler C(ST, Constants);
    try {
      C.compile(Program.get());
    } catch (const std::runtime_error &E) {
      std::cout << "compiler error: " << E.what() << "\n";
    }

    monkey::vm::VM Machine(C.byteCode(), Globals);
    try {
      Machine.run();
    } catch (const std::runtime_error &E) {
      std::cout << "vm error: " << E.what() << "\n";
    }

    Result = Machine.lastPoppedStackElem();
  } else {
    monkey::environment::Environment Env;
    ResultPtr = monkey::evaluator::eval(Program.get(), Env);
    Result = ResultPtr.get();
  }

  std::cout << "result is " << Result->inspect() << std::endl;
}
