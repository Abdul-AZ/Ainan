cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)

project(Ainan)

#define a new add_subdirectory function so that we have a cleaner solution explorer window in visual studio by using folders
################
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)
define_property(
    TARGET
    PROPERTY FOLDER
    INHERITED
    BRIEF_DOCS "Set the folder name."
    FULL_DOCS  "Use to organize targets in an IDE."
)

function(add_subdirectory_with_folder _folder_name _folder)
    add_subdirectory(${_folder} ${ARGN})
    set_property(DIRECTORY "${_folder}" PROPERTY FOLDER "${_folder_name}")
endfunction()
#################

##setup glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory_with_folder(glfw "submodules/glfw")
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
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_DRACO OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
add_subdirectory_with_folder(assimp "submodules/assimp")
list(APPEND STATIC_LIBRARIES assimp)

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
	message(FATAL_ERROR "Can't link with ffmpeg on platform")
endif()

add_subdirectory("src")