#include <pch.h>
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ALZ {

	bool Window::m_WindowSizeChanged = false;
	glm::vec2 Window::WindowSize = { 0,0 };
	GLFWwindow* Window::m_Window = nullptr;

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		Window::m_WindowSizeChanged = true;
		Window::WindowSize = { width, height };
		glViewport(0, 0, width, height);
	}

	void Window::Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_VERSION_MINOR, 0);

		m_Window = glfwCreateWindow(WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16, "ALZ Particles", nullptr, nullptr);
		WindowSize = { WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 };
		CenterWindow();

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(1);

		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Window::Present()
	{
		glfwSwapBuffers(m_Window);
		m_WindowSizeChanged = false;
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
		glfwDestroyWindow(m_Window);
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

	GLFWwindow & Window::GetWindow()
	{
		return *m_Window;
	}
}