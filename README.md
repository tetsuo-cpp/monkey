# Monkey
This project includes an interpreter, bytecode compiler and virtual machine for the Monkey programming language described in Thorsten Ball's books: "Writing an Interpreter In Go" and "Writing a Compiler In Go".
 language.
## Progress
* Interpreter (In Progress).
* Compiler.
* Virtual Machine.
## Dependencies
* CMake.
* Google Test.
* Google Mock.
## Build
Bring in Git submodules.
```
git submodule init
git submodule update
```
Use CMake to generate a platform specific build script and then invoke that.
```
cmake .
make
```
## Usage
Run the interpreter.
```
./monkey
```
Run the unit tests.
```
./monkey_test
```
