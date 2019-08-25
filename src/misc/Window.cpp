#include <pch.h>
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ALZ {

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

		Ptr = glfwCreateWindow(WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16, "ALZ Particles", nullptr, nullptr);
		FramebufferSize = { WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 };
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
		glfwGetWindowSize(&Window::GetWindow(), &windowWidth, &windowHeight);

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowPos(&Window::GetWindow(),
			monitorX + (mode->width - windowWidth) / 2,
			monitorY + (mode->height - windowHeight) / 2);
	}

	void Window::SetWindowLaunchSize()
	{
		glfwSetWindowSize(&Window::GetWindow(), WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16);
	}

	GLFWwindow& Window::GetWindow()
	{
		return *Ptr;
	}
}