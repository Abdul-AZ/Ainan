#include <pch.h>
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Ainan {

	class Renderer
	{
	public:
		static void SetViewport(const Rectangle& viewport);
		static void RecreateSwapchain(const glm::vec2& newSwapchainSize);
	};

	bool Window::ShouldClose = false;
	Rectangle Window::WindowViewport = { 0 };
	bool Window::WindowSizeChangedSinceLastFrame = false;
	glm::vec2 Window::FramebufferSize = { 0,0 };
	glm::vec2 Window::Size = { 0,0 };
	glm::vec2 Window::Position = { 0,0 };
	GLFWwindow* Window::Ptr = nullptr;

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		Window::WindowSizeChangedSinceLastFrame = true;
		Window::FramebufferSize = { width, height };
		Window::WindowViewport.Width = width;
		Window::WindowViewport.Height = height;

		Rectangle viewport{};
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = width;
		viewport.Height = height;
		Renderer::RecreateSwapchain(Window::FramebufferSize);
		Renderer::SetViewport(viewport);
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

	void Window::Init(RendererType api)
	{
#ifdef DEBUG
		glfwSetErrorCallback(error_callback);
#endif // !NDEBUG

		glfwInit();

		if (api == RendererType::OpenGL)
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // !NDEBUG
		}
		else
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		Ptr = glfwCreateWindow(c_StartMenuWidth, c_StartMenuHeight, "Ainan", nullptr, nullptr);
		int fbWidth = 0;
		int fbHeight = 0;
		glfwGetFramebufferSize(Ptr, &fbWidth, &fbHeight);
		FramebufferSize = { fbWidth, fbHeight };
		CenterWindow();

		glfwSetWindowSizeCallback(Ptr, window_size_callback);
		glfwSetFramebufferSizeCallback(Ptr, framebuffer_size_callback);
		glfwSetWindowPosCallback(Ptr, pos_callback);
	}

	void Window::HandleWindowEvents()
	{
		glfwPollEvents();
		ShouldClose = glfwWindowShouldClose(Ptr);
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

	void Window::SetSize(int32_t width, int32_t height)
	{
		glfwSetWindowSize(Ptr, width, height);
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