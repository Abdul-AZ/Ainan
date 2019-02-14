workspace "Simple-OpenGL-Particles"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }
    startproject "Core"

project "Core"

    kind ("ConsoleApp")
    cppdialect "C++17"

    files { "src/**.h", "src/**.hpp", "src/**.cpp", "src/**.c", "shaders/**.vert", "shaders/**.frag" }
    includedirs { "src"}

    links { "OpenGL32.lib" }

    filter "platforms:x64"
      links { "lib/glfw3_64.lib" }

    filter "platforms:x86"
      links { "lib/glfw3_32.lib" }

    filter "configurations:Debug"
      symbols "On"
      optimize "Off"

      filter "configurations:Release"
      symbols "Off"
      optimize "On"

    filter "system:Windows"
      systemversion ("latest")
