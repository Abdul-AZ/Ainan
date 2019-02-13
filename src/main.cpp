#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {


	glfwInit();

	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 0);

	GLFWwindow* window = nullptr;

	window = glfwCreateWindow(1000, 1000 * 9 / 16, "Particles", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	
	system("PAUSE");

	glfwTerminate();
}