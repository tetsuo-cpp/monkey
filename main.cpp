#include <REPL/REPL.h>

#include <iostream>

int main(int, char **) {
  std::cout << "Hello! This is the Monkey programming language.\n";
  std::cout << "Feel free to type in commands\n";

  monkey::REPL R;
  R.start();
}
