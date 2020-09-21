cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)

project(Ainan)

##setup glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("submodules/glfw")
set(STATIC_LIBRARIES glfw)

##setup glslcc (shader cross-compiler)
set(GLSLCC_BIN_PATH $<TARGET_FILE:glslcc>)
add_subdirectory("submodules/glslcc")
set(GLSL_SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders") ##to get the GLSL_SHADERS list
include("shaders/CMakeLists.txt")

##setup spdlog
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory("submodules/spdlog")
list(APPEND STATIC_LIBRARIES spdlog)

add_subdirectory("src")
