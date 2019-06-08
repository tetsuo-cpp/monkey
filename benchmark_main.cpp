#include <Compiler/Compiler.h>
#include <Evaluator/Evaluator.h>
#include <Lexer/Lexer.h>
#include <Parser/Parser.h>
#include <VM/VM.h>

#include <chrono>
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
                               "fibonacci(35);");

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: ./benchmark [engine]\n";
    return EXIT_FAILURE;
  }

  const std::string Engine(argv[1]);

  monkey::lexer::Lexer L(Input);
  monkey::parser::Parser P(L);
  auto Program = P.parseProgram();

  std::shared_ptr<monkey::object::Object> ResultPtr;
  const monkey::object::Object *Result;
  std::chrono::high_resolution_clock::time_point Start;
  std::chrono::high_resolution_clock::time_point End;

  if (Engine == "vm") {
    monkey::compiler::SymbolTable ST;
    std::vector<std::shared_ptr<monkey::object::Object>> Constants;
    std::array<std::shared_ptr<monkey::object::Object>, GLOBALS_SIZE> Globals;
    monkey::compiler::Compiler C(ST, Constants);

    try {
      C.compile(Program.get());
    } catch (const std::runtime_error &E) {
      std::cout << "compiler error: " << E.what() << "\n";
    }

    monkey::vm::VM Machine(C.byteCode(), Globals);
    Start = std::chrono::high_resolution_clock::now();

    try {
      Machine.run();
    } catch (const std::runtime_error &E) {
      std::cout << "vm error: " << E.what() << "\n";
    }

    End = std::chrono::high_resolution_clock::now();
    Result = Machine.lastPoppedStackElem();
  } else if (Engine == "eval") {
    auto Env = std::make_shared<monkey::environment::Environment>();
    Start = std::chrono::high_resolution_clock::now();

    ResultPtr = monkey::evaluator::eval(Program.get(), Env);

    End = std::chrono::high_resolution_clock::now();
    Result = ResultPtr.get();
  } else {
    std::cerr << "engine type must be one of [vm, eval]\n";
    return -1;
  }

  std::chrono::duration<double> Duration = End - Start;

  std::cout << "engine=" << Engine << ", result=" << Result->inspect()
            << ", duration=" << Duration.count() << "\n";
}
