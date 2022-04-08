cmake_minimum_required(VERSION 3.0.0)
project(pseudoblam)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/dependencies)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Assimp REQUIRED)

set(CGLM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/cglm/include/")

set(CGLTF_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/cgltf/")

file(GLOB_RECURSE GAME_C_SOURCE_FILES "game/source/*.c")
set(GAME_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/game/source/")

add_executable(pseudoblam ${GAME_C_SOURCE_FILES})
target_compile_options(pseudoblam PRIVATE -ansi -pedantic -Wall -Wextra -Wno-missing-braces)
target_compile_definitions(pseudoblam PRIVATE -DGL_SILENCE_DEPRECATION)
target_include_directories(pseudoblam PRIVATE ${GAME_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${CGLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(pseudoblam ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})

file(GLOB_RECURSE TOOLS_C_SOURCE_FILES "tools/source/*.c")
set(TOOLS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/tools/")

add_executable(tools ${TOOLS_C_SOURCE_FILES})
target_compile_options(tools PRIVATE -ansi -pedantic -Wall -Wextra -Wno-missing-braces)
target_include_directories(tools PRIVATE ${TOOLS_INCLUDE_DIRS} ${CGLM_INCLUDE_DIRS} ${CGLTF_INCLUDE_DIRS})
