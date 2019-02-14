#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ParticleSystem.h"

int main() {
	glfwInit();

	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 0);

	GLFWwindow* window = nullptr;

	window = glfwCreateWindow(1000, 1000 * 9 / 16, "Particles", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ParticleSystem Psystem;

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		Psystem.Draw();

		glfwSwapBuffers(window);

		glfwPollEvents();
		//Psystem.Update();
	}
	
	glfwTerminate();
}