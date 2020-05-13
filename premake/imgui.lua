project "imgui"
  basedir("../")
  kind ("StaticLib")
  objdir("%{wks.location}/build/obj/%{prj.name}_%{cfg.shortname}/")
  targetdir("%{wks.location}/build/bin/%{prj.name}_%{cfg.shortname}/")
  cppdialect "C++17"
  compileas "C++"

  files { 
    "../submodules/imgui/*.h", "../submodules/imgui/*.cpp",
    "../submodules/imgui/misc/cpp/imgui_stdlib.cpp", "../submodules/imgui/misc/cpp/imgui_stdlib.h"
   }

  includedirs { "../submodules/imgui", "../submodules/imgui/examples", "../submodules/imgui/misc/cpp", "../submodules/src/vendor" }

  defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD", "IMGUI_HAS_DOCK" }

  filter "configurations:Debug"
    runtime "Debug"
    symbols "On"
    optimize "Off"

  filter "configurations:Release"
    runtime "Release"
    symbols "Off"
    optimize "On"
    defines { "NDEBUG" }

  filter "system:Windows"
    systemversion ("latest")
    defines { "PLATFORM_WINDOWS" }

  filter "platforms:64bit"
    architecture "x64"

  filter "platforms:32bit"
    architecture "x32"