#pragma once

namespace ALZ {

	const float GlobalScaleFactor = 1000.0f;

	class Window
	{
	public:
		static void Init();
		static void Update();
		static void Present();
		static void Clear();
		static void Terminate();
		static bool WindowSizeChangedSinceLastFrame() { return m_WindowSizeChanged; };

		static GLFWwindow& GetWindow();
		static glm::vec2 WindowSize;

	private:
		static bool m_WindowSizeChanged;
		static GLFWwindow* m_Window;

		friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	};
}