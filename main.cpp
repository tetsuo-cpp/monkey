#include <REPL/REPL.h>

#include <iostream>
#include <stdlib.h>

int main(int, char **) {
  std::cout << "Hello " << std::getenv("USER")
            << "! This is the Monkey programming language!\n";
  std::cout << "Feel free to type in commands\n";

  monkey::repl::REPL R;
  R.start();
}
