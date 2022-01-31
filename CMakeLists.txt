cmake_minimum_required(VERSION 3.0.0)
project(c-language-prototypes)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

file(GLOB_RECURSE PROJECT_C_SOURCE_FILES source/*.c)
set(PROJECT_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/source")

add_executable(c-language-prototypes ${PROJECT_C_SOURCE_FILES})
target_compile_options(c-language-prototypes PRIVATE -ansi -pedantic -Wall -Wextra -Werror)
target_include_directories(c-language-prototypes PRIVATE ${PROJECT_INCLUDE_DIRS})
