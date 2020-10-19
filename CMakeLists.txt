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

#link with ffmpeg
if(WIN32)
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avcodec.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avutil.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avresample.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/swresample.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avformat.lib")
	list(APPEND STATIC_LIBRARIES "bcrypt.lib")
	list(APPEND STATIC_LIBRARIES "mfplat.lib")
	list(APPEND STATIC_LIBRARIES "strmiids.lib")
	list(APPEND STATIC_LIBRARIES "ole32.lib")
	list(APPEND STATIC_LIBRARIES "mfuuid.lib")
else()
	message(FATAL_ERROR "Can't link with ffmpeg on platform")
endif()

add_subdirectory("src")
