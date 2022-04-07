cmake_minimum_required(VERSION 3.0.0)
project(pseudoblam)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/dependencies)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Assimp REQUIRED)

file(GLOB_RECURSE PROJECT_C_SOURCE_FILES source/*.c)
set(PROJECT_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/source")

set(CGLM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/cglm/include/")

add_executable(pseudoblam ${PROJECT_C_SOURCE_FILES})
target_compile_options(pseudoblam PRIVATE -ansi -pedantic -Wall -Wextra -Wno-missing-braces)
target_compile_definitions(pseudoblam PRIVATE -DGL_SILENCE_DEPRECATION)
target_include_directories(pseudoblam PRIVATE ${PROJECT_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${CGLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(pseudoblam ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
