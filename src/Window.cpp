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

	m_Window = glfwCreateWindow(1500, 1500 * 9 / 16, "Particles", nullptr, nullptr);
	m_ScreenSize = { 1500, 1500 * 9 / 16 };

	glfwMakeContextCurrent(m_Window);

	glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Window::Update()
{

	glfwPollEvents();
}

void Window::Present()
{
	glfwSwapBuffers(m_Window);
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

GLFWwindow & Window::GetWindow()
{
	return *m_Window;
}

glm::vec2 & Window::GetSize()
{
	return m_ScreenSize;
}
