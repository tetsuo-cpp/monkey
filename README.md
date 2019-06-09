# Monkey
This project includes C++ implementations of an interpreter, bytecode compiler and virtual machine for the Monkey programming language described in Thorsten Ball's books: "Writing an Interpreter In Go" and "Writing a Compiler In Go".
## Progress
* Interpreter (Finished).
* Compiler (Finished).
* Virtual Machine (Finished).
## Dependencies
* CMake.
* Google Test.
* Google Mock.
* Boost Pool.
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
Run the fibonacci program.
```
./benchmark [vm/eval]
```
## Notes
This repository is more or less a word for word C++ translation of the Go code presented in Thorsten Ball's books. As such, a lot of the code is unidiomatic or suboptimal for a C++ program.

The Go code does the equivalent of `dynamic_cast` a lot. To avoid that, I call a virtual function to check an enum value representing the underlying type and then use `static_cast` to safely and quickly downcast. See `astCast` and `objCast` for this. I think we usually prefer the visitor pattern for this type of thing.

Another example is my use of `std::shared_ptr` (the STL's ref-counted pointer implementation) to emulate Go's garbage collector. This made sense to me since I wanted to keep the class hierarchy of Object the same as in the book and this technique didn't cause the code structure to deviate too much from the Go code. The downsides are that, despite preallocating the stack in the VM, object creation involves additional dynamic allocations and in multi-threaded programs the copying of a `std::shared_ptr` uses (relatively) expensive atomic operations. As an easy and unintrusive fix, I used Boost's Object Pool for quick allocations and made sure that libcxx doesn't use atomic operations for `std::shared_ptr` copying since Monkey is a single threaded program (libstdc++ seemed to do the right thing by default).

I was thinking that another approach would be to not access objects through this Object interface and instead use a `std::variant` that includes the different possible object types. Since objects in Monkey are immutable (for example, the `push` built-in actually creates a new list with the new element appended to it) you can simply assign the `std::variant` to its preallocated spot on the stack. It would involve more copying of the object data but I would expect the saving of dynamic allocations and improved cache locality to dwarf the effect of increased copies. You'd maybe need to make an exception for strings, lists and hash maps since if most of your memory is used by a single one of these then doubling the usage with a copy probably wouldn't be acceptable.

Another intrusive alternative would be to use a real garbage collector like Boehm instead of hacking around the problem with `std::shared_ptr`.

Overall this has been a fun project and I recommend these books for people interested in learning about compilers. The books are practically focused and don't expect any prior knowledge of compilers or CS theory. This is just the beginning of my compiler studies so hopefully when I'm older and wiser I can come back and add some interesting ideas to this notes section.