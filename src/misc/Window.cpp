#include <pch.h>
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ALZ {

	bool Window::ShouldClose = false;
	Viewport Window::WindowViewport = { 0 };
	bool Window::WindowSizeChangedSinceLastFrame = false;
	glm::vec2 Window::FramebufferSize = { 0,0 };
	glm::vec2 Window::Size = { 0,0 };
	glm::vec2 Window::Position = { 0,0 };
	GLFWwindow* Window::Ptr = nullptr;

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		Window::WindowSizeChangedSinceLastFrame = true;
		Window::FramebufferSize = { width, height };
		Window::WindowViewport.width = width;
		Window::WindowViewport.height = height;
		glViewport(0, 0, width, height);
	}

	void window_size_callback(GLFWwindow* window, int width, int height)
	{
		Window::Size.x = width;
		Window::Size.y = height;
	}

	void pos_callback(GLFWwindow* window, int x, int y)
	{
		Window::Position.x = x;
		Window::Position.y = y;
	}

	void error_callback(int num, const char* message)
	{
		std::cout << message << std::endl;
	}

	void Window::Init()
	{
#ifdef DEBUG
		glfwSetErrorCallback(error_callback);
#endif // !NDEBUG

		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // !NDEBUG

		Ptr = glfwCreateWindow(500, 500 * 9 / 16, "ALZ Particles", nullptr, nullptr);
		int fbWidth = 0;
		int fbHeight = 0;
		glfwGetFramebufferSize(Ptr, &fbWidth, &fbHeight);
		FramebufferSize = { fbWidth, fbHeight };
		CenterWindow();

		glfwMakeContextCurrent(Ptr);
		glfwSwapInterval(1);

		glfwSetWindowSizeCallback(Ptr, window_size_callback);
		glfwSetFramebufferSizeCallback(Ptr, framebuffer_size_callback);
		glfwSetWindowPosCallback(Ptr, pos_callback);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Window::Present()
	{
		glfwSwapBuffers(Ptr);
		WindowSizeChangedSinceLastFrame = false;
	}

	void Window::HandleWindowEvents()
	{
		glfwPollEvents();
		ShouldClose = glfwWindowShouldClose(Ptr);
	}

	void Window::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::Terminate()
	{
		glfwDestroyWindow(Ptr);
		glfwTerminate();
	}

	void Window::CenterWindow()
	{
		int count;
		GLFWmonitor* monitor = glfwGetMonitors(&count)[0];
		int monitorX, monitorY;
		glfwGetMonitorPos(monitor, &monitorX, &monitorY);

		int windowWidth, windowHeight;
		glfwGetWindowSize(Ptr, &windowWidth, &windowHeight);

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowPos(Ptr,
			monitorX + (mode->width - windowWidth) / 2,
			monitorY + (mode->height - windowHeight) / 2);
	}

	void Window::Restore()
	{
		glfwRestoreWindow(Ptr);
	}

	void Window::Maximize()
	{
		glfwMaximizeWindow(Ptr);
	}

	void Window::SetSize(const glm::ivec2& size)
	{
		glfwSetWindowSize(Ptr, size.x, size.y);
	}

	void Window::SetShouldClose()
	{
		glfwSetWindowShouldClose(Ptr, true);
	}

	void Window::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(Ptr, title.c_str());
	}
}