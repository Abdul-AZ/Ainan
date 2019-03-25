project "imgui"
kind ("StaticLib")
basedir("../")

cppdialect "C++17"

files { 
  "../imgui/*.h", "../imgui/*.cpp",
  "../imgui/examples/imgui_impl_opengl3.h", "../imgui/examples/imgui_impl_opengl3.cpp",
  "../imgui/examples/imgui_impl_glfw.h", "../imgui/examples/imgui_impl_glfw.cpp",
  "../imgui/misc/cpp/imgui_stdlib.cpp", "../imgui/misc/cpp/imgui_stdlib.h"
 }

includedirs { "../imgui", "../imgui/examples", "../imgui/misc/cpp", "../src" }

defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD", "IMGUI_HAS_DOCK" }

filter "configurations:Debug"
  symbols "On"
  optimize "Off"

  filter "configurations:Release"
  symbols "Off"
  optimize "On"
  defines { "NDEBUG" }

filter "system:Windows"
  systemversion ("latest")