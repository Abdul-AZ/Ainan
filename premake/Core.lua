project "Core"
  basedir("../")
  kind("ConsoleApp")
  objdir("%{wks.location}/build/obj/%{prj.name}_%{cfg.shortname}/")
  targetdir("%{wks.location}/build/bin/%{prj.name}_%{cfg.shortname}/")
  cppdialect "C++17"
  compileas "C++"

  files { 
    "../src/**.h", "../src/**.hpp", "../src/**.cpp", "../src/**.c", "../shaders/**.vert", "../shaders/**.frag",
   }

  pchheader "pch.h"
  pchsource "../src/pch.cpp"

  includedirs { "../src", "../submodules/imgui", "../submodules/imgui/examples", "../submodules/imgui/misc/cpp", "../submodules/glm", "../src/vendor" }

  links { "OpenGL32.lib", "glfw", "imgui" }

  filter "configurations:Debug"
    runtime "Debug"
    symbols "On"
    optimize "Off"
	defines { "DEBUG" }

  filter "configurations:Release"
    runtime "Release"
    symbols "On"
    optimize "On"
	defines { "NDEBUG" }

  filter "system:Windows"
    systemversion ("latest")
    links { "d3d11.lib" }
    defines { "PLATFORM_WINDOWS" }

  filter "platforms:64bit"
    architecture "x64"

  filter "platforms:32bit"
    architecture "x32"