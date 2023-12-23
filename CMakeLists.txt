cmake_minimum_required(VERSION 3.5.0)
project(pseudoblam)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Assimp REQUIRED)

# https://github.com/recp/cglm
set(CGLM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/cglm/include/")

# https://github.com/jkuhlmann/cgltf
set(CGLTF_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/cgltf/")

# https://github.com/nothings/stb
set(STB_IMAGE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/stb/")

file(GLOB_RECURSE SHARED_C_SOURCE_FILES "shared/source/*.c")
set(SHARED_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/shared/source/")
add_library(shared STATIC ${SHARED_C_SOURCE_FILES})
target_include_directories(shared PUBLIC ${SHARED_INCLUDE_DIRS})

file(GLOB_RECURSE GAME_C_SOURCE_FILES "game/source/*.c")
set(GAME_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/game/source/")
add_executable(game ${GAME_C_SOURCE_FILES})
target_compile_options(game PRIVATE -ansi -Wall -Wextra -std=gnu2x)
target_compile_definitions(game PRIVATE -DGL_SILENCE_DEPRECATION)
target_include_directories(game PRIVATE ${GAME_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${CGLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(game ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES} shared)

file(GLOB_RECURSE TOOLS_C_SOURCE_FILES "tools/source/*.c")
set(TOOLS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/tools/source/")
add_executable(tools ${TOOLS_C_SOURCE_FILES})
target_compile_options(tools PRIVATE -ansi -Wall -Wextra -std=gnu2x)
target_include_directories(tools PRIVATE ${TOOLS_INCLUDE_DIRS} ${CGLM_INCLUDE_DIRS} ${CGLTF_INCLUDE_DIRS} ${STB_IMAGE_INCLUDE_DIRS})
target_link_libraries(tools shared)
