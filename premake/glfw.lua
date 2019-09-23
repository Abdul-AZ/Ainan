project "glfw"
  kind ("StaticLib")

  basedir("../")
  files {
    "../submodules/glfw/src/init.c",
    "../submodules/glfw/src/input.c",
    "../submodules/glfw/src/window.c",
    "../submodules/glfw/src/internal.h",
    "../submodules/glfw/src/monitor.c",
    "../submodules/glfw/src/vulkan.c",
    "../submodules/glfw/src/wgl_context.c",
    "../submodules/glfw/src/wgl_context.h",
    "../submodules/glfw/src/egl_context.c",
    "../submodules/glfw/src/egl_context.h",
    "../submodules/glfw/src/osmesa_context.c",
    "../submodules/glfw/src/osmesa_context.h",
    "../submodules/glfw/src/context.c"
  }

  filter "system:Windows"
   files {
     "../submodules/glfw/src/win32_platform.h",
     "../submodules/glfw/src/win32_init.c",
     "../submodules/glfw/src/win32_monitor.c",
     "../submodules/glfw/src/win32_thread.c",
     "../submodules/glfw/src/win32_time.c",
     "../submodules/glfw/src/win32_joystick.c",
     "../submodules/glfw/src/win32_joystick.h",
     "../submodules/glfw/src/win32_window.c"
   }
   systemversion ("latest")
   defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }

  filter "platforms:64bit"
    architecture "x64"

  filter "platforms:32bit"
    architecture "x32"