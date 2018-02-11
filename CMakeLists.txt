cmake_minimum_required(VERSION 3.7)
project(voxel)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    gl3w/gl3w.h
    gl3w/gl3w.c
    gl3w/glcorearb.h
    src/main.cpp
    src/Input.hpp
    src/Player.hpp
    src/Shader.hpp
    src/Shader.cpp
    src/VoxelStorage.hpp
    src/VoxelStorage.cpp
    src/Window.hpp
    src/VoxelScene.hpp
    src/VoxelScene.cpp
    src/MeshIterator.hpp
    src/QuadEBO.hpp
)

add_executable(voxel ${SOURCE_FILES})

# glfw3
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(voxel ${GLFW_LIBRARIES})

# pthread
target_link_libraries(voxel pthread)

# zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(voxel ${ZLIB_LIBRARIES})

# dl needed by gl3w
target_link_libraries(voxel ${CMAKE_DL_LIBS})
