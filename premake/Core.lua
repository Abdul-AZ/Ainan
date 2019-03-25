project "Core"

basedir("../")
kind ("ConsoleApp")
cppdialect "C++17"
compileas "C++"

files { 
  --project files
  "../src/**.h", "../src/**.hpp", "../src/**.cpp", "../src/**.c", "../shaders/**.vert", "../shaders/**.frag",
  --imgui files
  "../imgui/*.h", "../imgui/*.cpp",
  "../imgui/examples/imgui_impl_opengl3.h", "../imgui/examples/imgui_impl_opengl3.cpp",
  "../imgui/examples/imgui_impl_glfw.h", "../imgui/examples/imgui_impl_glfw.cpp",
  "../imgui/misc/cpp/imgui_stdlib.cpp", "../imgui/misc/cpp/imgui_stdlib.h",
  --glm files
  "glm/glm/**.hpp"
 }

pchheader "pch.h"
pchsource "../src/pch.cpp"

includedirs { "../src", "../imgui", "../imgui/examples", "../imgui/misc/cpp", "../glm" }

defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD", "IMGUI_HAS_DOCK" }

links { "OpenGL32.lib", "glfw" }

filter "configurations:Debug"
  symbols "On"
  optimize "Off"

  filter "configurations:Release"
  symbols "Off"
  optimize "On"
  defines { "NDEBUG" }

filter "system:Windows"
  systemversion ("latest")