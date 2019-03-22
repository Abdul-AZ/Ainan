#pragma once

static class Window 
{
public:
	static void Init();
	static void Update();
	static void Present();
	static void Clear();
	static void Terminate();

	static GLFWwindow& GetWindow();
	static glm::vec2& GetSize();

private:
	static glm::vec2 m_ScreenSize;
	static GLFWwindow* m_Window;

	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};
