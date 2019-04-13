cmake_minimum_required(VERSION 3.9)
project(monkey)

# Compiler flags.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# Setup GTest/GMock.
add_subdirectory(
  "${PROJECT_SOURCE_DIR}/external/googletest"
  "external/googletest"
  )

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Build interpreter binary.
set(
  MONKEY_SOURCE_FILES
  AST/AST.cpp
  Code/Code.cpp
  Compiler/Compiler.cpp
  Environment/Environment.cpp
  Evaluator/Evaluator.cpp
  Lexer/Lexer.cpp
  Object/Object.cpp
  Parser/Parser.cpp
  REPL/REPL.cpp
  Token/Token.cpp
  main.cpp
  )

add_executable(monkey ${MONKEY_SOURCE_FILES})
target_include_directories(monkey PRIVATE .)

# Build unit test binary.
set(
  MONKEY_TEST_SOURCE_FILES
  AST/AST.cpp
  AST/ASTTest.cpp
  Code/Code.cpp
  Code/CodeTest.cpp
  Compiler/Compiler.cpp
  Compiler/CompilerTest.cpp
  Environment/Environment.cpp
  Evaluator/Evaluator.cpp
  Evaluator/EvaluatorTest.cpp
  Lexer/Lexer.cpp
  Lexer/LexerTest.cpp
  Object/Object.cpp
  Parser/Parser.cpp
  Parser/ParserTest.cpp
  Token/Token.cpp
  test_main.cpp
  )

add_executable(monkey_test ${MONKEY_TEST_SOURCE_FILES})

target_link_libraries(monkey_test gtest gmock)
target_include_directories(monkey_test PRIVATE .)
