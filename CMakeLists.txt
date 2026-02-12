####################
# Config

cmake_minimum_required(VERSION 4.1)
project(voxel_game VERSION 1.0 LANGUAGES CXX)
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
        src/main.cpp
        src/gfx/vertex_array.cpp
        src/gfx/vertex_buffer.cpp
        src/gfx/index_buffer.cpp
        src/gfx/drawing.cpp
        src/gfx/shader.cpp
        src/rend/camera.cpp
)


####################

# Build it
add_executable(voxel_game ${SOURCES})
target_link_libraries(voxel_game PRIVATE glfw glm glbinding)
