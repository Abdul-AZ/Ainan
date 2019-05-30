project "imgui"
kind ("StaticLib")
basedir("../")

cppdialect "C++17"

files { 
  "../imgui/*.h", "../imgui/*.cpp",
  "../imgui/misc/cpp/imgui_stdlib.cpp", "../imgui/misc/cpp/imgui_stdlib.h"
 }

includedirs { "../imgui", "../imgui/examples", "../imgui/misc/cpp", "../src/vendor" }

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