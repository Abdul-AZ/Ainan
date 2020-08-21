project "Core"
  basedir("../")
  kind("ConsoleApp")
  objdir("%{wks.location}/build/obj/%{prj.name}_%{cfg.shortname}/")
  targetdir("%{wks.location}/build/bin/%{prj.name}_%{cfg.shortname}/")
  cppdialect "C++17"
  compileas "C++"

  files { 
    --c and c++ code
    "../src/**.h", "../src/**.hpp", "../src/**.cpp", "../src/**.c",
    --glsl code
    "../shaders/**.vert", "../shaders/**.frag"
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
    links { "d3d11.lib", "d3dcompiler.lib" }
    defines { "PLATFORM_WINDOWS" }
    files { "../shaders/**.hlsl", "../shaders/**.hlsl" }

filter { "files:../shaders/**-v.hlsl" }
   shadertype "Vertex"
   shaderentry "main"
   shadermodel "5.0"
   shaderobjectfileoutput("%{prj.location}/shaders/%{file.basename}.cso")

filter { "files:../shaders/**-f.hlsl" }
   shadertype "Pixel"
   shaderentry "main"
   shadermodel "5.0"
   shaderobjectfileoutput("%{prj.location}/shaders/%{file.basename}.cso")

  filter "platforms:64bit"
    architecture "x64"

  filter "platforms:32bit"
    architecture "x32"