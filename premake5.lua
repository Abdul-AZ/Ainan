workspace "ALZparticles"
    configurations { "Debug", "Release" }
    platforms { "64bit", "32bit" }
    startproject "Core"

include "premake/Core.lua"
include "premake/glfw.lua"
include "premake/imgui.lua"
