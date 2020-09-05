cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)

project(Ainan)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("submodules/glfw")
set(STATIC_LIBRARIES glfw)

add_subdirectory("src")