# Monkey
Since I've been wanting to learn about compilers, I recently picked up copies of Thorsten Ball's books "Writing an Interpreter In Go" and "Writing a Compiler In Go".
By the end of the series, I should have a working interpreter, bytecode compiler and virtual machine for the Monkey programming language.

I've tried to keep my C++ as a relatively direct translation of the author's Go code so it may look unidiomatic in some places.
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
# Usage
Run the interpreter.
```
./monkey
```
Run the unit tests.
```
./monkey_test
```
