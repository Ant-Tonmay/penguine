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
# Source files
# -----------------------------
file(GLOB_RECURSE PENGUIN_SOURCES
    src/*.cpp
)
list(FILTER PENGUIN_SOURCES EXCLUDE REGEX "src/test_lexer.cpp")

# -----------------------------
# Executable
# -----------------------------
add_executable(penguin ${PENGUIN_SOURCES})

# -----------------------------
# Tests
# -----------------------------
enable_testing()
add_executable(penguin_test src/test_lexer.cpp src/lexer/lexer.cpp)
target_include_directories(penguin_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME LexerTest COMMAND penguin_test)


# -----------------------------
# Include directories
# -----------------------------
target_include_directories(penguin
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------
# Compiler warnings (very important)
# -----------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(penguin PRIVATE
        -Wall -Wextra -Wpedantic
    )
elseif (MSVC)
    target_compile_options(penguin PRIVATE
        /W4
    )
endif()

# -----------------------------
# Debug flags
# -----------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(penguin PRIVATE DEBUG)
endif()
