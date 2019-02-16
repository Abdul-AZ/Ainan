#include "Window.h"

glm::vec2 Window::m_ScreenSize = { 0,0 };
GLFWwindow* Window::m_Window = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Window::m_ScreenSize = { width, height };
	glViewport(0, 0, width, height);
}

void Window::Init()
{
	glfwInit();

	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 0);

	m_Window = glfwCreateWindow(1000, 1000 * 9 / 16, "Particles", nullptr, nullptr);
	m_ScreenSize = { 1000, 1000 * 9 / 16 };

	glfwMakeContextCurrent(m_Window);

	glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
}

void Window::Update()
{

	glfwPollEvents();
}

void Window::Present()
{
	glfwSwapBuffers(m_Window);
}

GLFWwindow & Window::GetWindow()
{
	return *m_Window;
}

glm::vec2 & Window::GetSize()
{
	return m_ScreenSize;
}
