cmake_minimum_required(VERSION 3.16)

project(
    penguin
    VERSION 0.1.0
    DESCRIPTION "Penguin Programming Language"
    LANGUAGES CXX
)

# -----------------------------
# C++ standard
# -----------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Core library
# -----------------------------
add_library(penguin_core
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/interpreter/interpreter.cpp
    src/symbol_table/symbol_table.cpp
)

target_include_directories(penguin_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------
# Compiler warnings
# -----------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(penguin_core PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(penguin_core PRIVATE /W4)
endif()

# -----------------------------
# Main executable
# -----------------------------
add_executable(penguin src/main.cpp)
target_link_libraries(penguin PRIVATE penguin_core)

# -----------------------------
# Tests
# -----------------------------
enable_testing()

add_executable(penguin_lexer_test src/test_lexer.cpp)
target_link_libraries(penguin_lexer_test PRIVATE penguin_core)
add_test(NAME LexerTest COMMAND penguin_lexer_test)

add_executable(penguin_parser_test src/test_parser.cpp)
target_link_libraries(penguin_parser_test PRIVATE penguin_core)
add_test(NAME ParserTest COMMAND penguin_parser_test)

add_executable(penguin_symbol_table_test src/test_symbol_table.cpp)
target_link_libraries(penguin_symbol_table_test PRIVATE penguin_core)
add_test(NAME SymbolTableTest COMMAND penguin_symbol_table_test)

add_executable(penguin_interpreter_test src/test_interpreter.cpp)
target_link_libraries(penguin_interpreter_test PRIVATE penguin_core)
add_test(NAME InterpreterTest COMMAND penguin_interpreter_test)

# -----------------------------
# Debug flags
# -----------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(penguin_core PRIVATE DEBUG)
endif()
