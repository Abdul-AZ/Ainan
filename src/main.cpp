#include <iostream>
#include <chrono>
#include <ctime>

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

	std::clock_t timeStart = 0;
	std::clock_t timeEnd = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window))
	{
		timeEnd = clock();
		float deltaTime = (timeEnd - timeStart) / 1000.0f;
		timeStart = timeEnd;

		//Update
		glfwPollEvents();
		Psystem.Update(deltaTime);
		
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			Particle particle(glm::vec2(xpos / Psystem.Scale,ypos / Psystem.Scale),glm::vec4(1.0f,1.0f,1.0f,1.0));

			particle.m_Velocity = glm::vec2((rand() % 21) - 10, (rand() % 21) - 10);

			Psystem.SpawnParticle(particle);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			Psystem.ClearParticles();
		//Render
		glClear(GL_COLOR_BUFFER_BIT);
		Psystem.Draw();
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
}