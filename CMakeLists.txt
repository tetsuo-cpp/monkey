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

# Build unit test binary.
set(
  MONKEY_TEST_SOURCE_FILES
  lib/Lexer/Lexer.cpp
  lib/Lexer/LexerTest.cpp
  test/test_main.cpp
  )

add_executable(monkey_test ${MONKEY_TEST_SOURCE_FILES})

target_link_libraries(monkey_test gtest gmock)
target_include_directories(monkey_test PRIVATE lib)
