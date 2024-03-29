cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0077 NEW)

project(Ainan)

#define a new function to add projects and put them in a folder in the visual studio solution explorer
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)
include("cmake/add_subdirectory_with_folder.cmake")

#setup glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory_with_folder(glfw "submodules/glfw")
set_target_properties(glfw PROPERTIES FOLDER glfw) #annoying workaround because glfw sets its own GLFW3 folder in visual studio
set(STATIC_LIBRARIES glfw)

##setup glslcc (shader cross-compiler)
set(GLSLCC_BIN_PATH $<TARGET_FILE:glslcc>)
add_subdirectory_with_folder(glslcc "submodules/glslcc")
set(GLSL_SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders") ##to get the GLSL_SHADERS list
include("shaders/CMakeLists.txt")

##setup spdlog
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
add_subdirectory_with_folder(spdlog "submodules/spdlog")
list(APPEND STATIC_LIBRARIES spdlog)

##setup assimp
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory_with_folder(assimp "submodules/assimp")
list(APPEND STATIC_LIBRARIES assimp)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
#link with ffmpeg
if(WIN32)
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avcodec.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avutil.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avresample.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/swresample.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/avformat.lib")
	list(APPEND STATIC_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/swscale.lib")
	list(APPEND STATIC_LIBRARIES "bcrypt.lib")
	list(APPEND STATIC_LIBRARIES "mfplat.lib")
	list(APPEND STATIC_LIBRARIES "strmiids.lib")
	list(APPEND STATIC_LIBRARIES "ole32.lib")
	list(APPEND STATIC_LIBRARIES "mfuuid.lib")
	list(APPEND STATIC_LIBRARIES "secur32.lib")
	list(APPEND STATIC_LIBRARIES "ws2_32.lib")
else()
	find_package(FFmpeg REQUIRED)
	list(APPEND STATIC_LIBRARIES ${FFMPEG_LIBRARIES})
endif()

add_subdirectory("src")
