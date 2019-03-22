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
      "imgui/examples/imgui_impl_glfw.h", "imgui/examples/imgui_impl_glfw.cpp",
      "imgui/misc/cpp/imgui_stdlib.cpp", "imgui/misc/cpp/imgui_stdlib.h",
      --glm files
      "glm/glm/**.hpp"
     }

    includedirs { "src", "imgui", "imgui/examples", "imgui/misc/cpp", "glm" }

    defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD", "IMGUI_HAS_DOCK" }

    links { "OpenGL32.lib", "glfw" }

    filter "configurations:Debug"
      symbols "On"
      optimize "Off"

      filter "configurations:Release"
      symbols "Off"
      optimize "On"

    filter "system:Windows"
      systemversion ("latest")


      project "glfw"
      kind ("StaticLib")
      files {
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/window.c",
        "glfw/src/internal.h",
        "glfw/src/monitor.c",
        "glfw/src/vulkan.c",
        "glfw/src/wgl_context.c",
        "glfw/src/wgl_context.h",
        "glfw/src/egl_context.c",
        "glfw/src/egl_context.h",
        "glfw/src/osmesa_context.c",
        "glfw/src/osmesa_context.h",
        "glfw/src/context.c"
      }
  
      filter "system:Windows"
       files {
         "glfw/src/win32_platform.h",
         "glfw/src/win32_init.c",
         "glfw/src/win32_monitor.c",
         "glfw/src/win32_thread.c",
         "glfw/src/win32_time.c",
         "glfw/src/win32_joystick.c",
         "glfw/src/win32_joystick.h",
         "glfw/src/win32_window.c"
       }
       systemversion ("latest")
       defines { "_GLFW_WIN32" }
  
