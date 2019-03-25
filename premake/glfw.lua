project "glfw"
kind ("StaticLib")
basedir("../")
files {
  "../glfw/src/init.c",
  "../glfw/src/input.c",
  "../glfw/src/window.c",
  "../glfw/src/internal.h",
  "../glfw/src/monitor.c",
  "../glfw/src/vulkan.c",
  "../glfw/src/wgl_context.c",
  "../glfw/src/wgl_context.h",
  "../glfw/src/egl_context.c",
  "../glfw/src/egl_context.h",
  "../glfw/src/osmesa_context.c",
  "../glfw/src/osmesa_context.h",
  "../glfw/src/context.c"
}

filter "system:Windows"
 files {
   "../glfw/src/win32_platform.h",
   "../glfw/src/win32_init.c",
   "../glfw/src/win32_monitor.c",
   "../glfw/src/win32_thread.c",
   "../glfw/src/win32_time.c",
   "../glfw/src/win32_joystick.c",
   "../glfw/src/win32_joystick.h",
   "../glfw/src/win32_window.c"
 }
 systemversion ("latest")
 defines { "_GLFW_WIN32" }
