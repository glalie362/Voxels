####################
# Config

cmake_minimum_required(VERSION 4.1)
project(cyrex_voxels VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 26)
include(FetchContent)

#####################
# Libs
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        glbinding
        GIT_REPOSITORY https://github.com/cginternals/glbinding.git
        GIT_TAG        master # or a specific release tag
)
FetchContent_MakeAvailable(glbinding)


FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)


#####################

set(SOURCES
        src/gfx/vertex_array.cpp
        src/gfx/vertex_buffer.cpp
        src/gfx/index_buffer.cpp
        src/gfx/drawing.cpp
        src/gfx/shader.cpp
        src/rend/camera.cpp
)


####################

# Build it
add_library(cyrex_voxels ${SOURCES})
target_link_libraries(cyrex_voxels PRIVATE glfw glm glbinding)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(cyrex_voxels PRIVATE -O3 -march=native -flto)
endif()