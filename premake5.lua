workspace "Simple-OpenGL-Particles"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }
    startproject "Core"

include "premake/Core.lua"
include "premake/glfw.lua"
include "premake/imgui.lua"
