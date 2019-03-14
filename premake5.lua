workspace "Simple-OpenGL-Particles"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }
    startproject "Core"

project "Core"

    kind ("ConsoleApp")
    cppdialect "C++17"

    files { 
      --project files
      "src/**.h", "src/**.hpp", "src/**.cpp", "src/**.c", "shaders/**.vert", "shaders/**.frag",
      --imgui files
      "imgui/*.h", "imgui/*.cpp",
      "imgui/examples/imgui_impl_opengl3.h", "imgui/examples/imgui_impl_opengl3.cpp",
      "imgui/examples/imgui_impl_glfw.h", "imgui/examples/imgui_impl_glfw.cpp"
     }

    includedirs { "src", "imgui", "imgui/examples"}

    defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD", "IMGUI_HAS_DOCK" }

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
