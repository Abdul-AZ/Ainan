project "Core"

basedir("../")
kind ("ConsoleApp")
cppdialect "C++17"
compileas "C++"

files { 
  --project files
  "../src/**.h", "../src/**.hpp", "../src/**.cpp", "../src/**.c", "../shaders/**.vert", "../shaders/**.frag",
  --glm files
  "glm/glm/**.hpp"
 }

pchheader "pch.h"
pchsource "../src/pch.cpp"

includedirs { "../src", "../imgui", "../imgui/examples", "../imgui/misc/cpp", "../glm", "../src/vendor" }

defines { "_CRT_SECURE_NO_WARNINGS" }

links { "OpenGL32.lib", "glfw", "imgui" }

filter "configurations:Debug"
  symbols "On"
  optimize "Off"

  filter "configurations:Release"
  symbols "Off"
  optimize "On"
  defines { "NDEBUG" }

filter "system:Windows"
  systemversion ("latest")