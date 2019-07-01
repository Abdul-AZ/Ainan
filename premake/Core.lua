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
    runtime "Debug"
    symbols "On"
    optimize "Off"

  filter "configurations:Release"
    runtime "Release"
    symbols "On"
    optimize "Off"

  filter "system:Windows"
    systemversion ("latest")

  filter "platforms:64bit"
    architecture "x64"

  filter "platforms:32bit"
    architecture "x32"