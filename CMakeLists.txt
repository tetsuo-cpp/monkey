cmake_minimum_required(VERSION 3.9)
project(monkey)

# Compiler flags.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Setup GTest/GMock.
add_subdirectory(
  "${PROJECT_SOURCE_DIR}/external/googletest"
  "external/googletest"
  )

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
add_definitions(-D_LIBCPP_HAS_NO_THREADS -DBOOST_DISABLE_THREADS)

# Build monkey lib.
set(
  MONKEY_LIB_FILES
  AST/AST.cpp
  Code/Code.cpp
  Compiler/Compiler.cpp
  Compiler/SymbolTable.cpp
  Environment/Environment.cpp
  Evaluator/Evaluator.cpp
  Lexer/Lexer.cpp
  Object/BuiltIns.cpp
  Object/Object.cpp
  Parser/Parser.cpp
  REPL/REPL.cpp
  Token/Token.cpp
  VM/Frame.cpp
  VM/VM.cpp
  main.cpp
  )

add_library(monkey_lib ${MONKEY_LIB_FILES})
target_include_directories(monkey_lib PRIVATE .)

# Build interpreter binary.
set(
  MONKEY_SOURCE_FILES
  main.cpp
  )

add_executable(monkey ${MONKEY_SOURCE_FILES})
target_link_libraries(monkey monkey_lib)
target_include_directories(monkey PRIVATE .)

# Build unit test binary.
set(
  MONKEY_TEST_SOURCE_FILES
  AST/ASTTest.cpp
  Code/CodeTest.cpp
  Compiler/CompilerTest.cpp
  Compiler/SymbolTableTest.cpp
  Evaluator/EvaluatorTest.cpp
  Lexer/LexerTest.cpp
  Parser/ParserTest.cpp
  VM/VMTest.cpp
  test_main.cpp
  )

add_executable(monkey_test ${MONKEY_TEST_SOURCE_FILES})

target_link_libraries(monkey_test monkey_lib gtest gmock)
target_include_directories(monkey_test PRIVATE .)

# Build the benchmark binary.
set(
  BENCHMARK_SOURCE_FILES
  benchmark_main.cpp
  )

add_executable(benchmark ${BENCHMARK_SOURCE_FILES})
target_link_libraries(benchmark monkey_lib)
target_include_directories(benchmark PRIVATE .)
