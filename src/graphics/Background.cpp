#include <pch.h>

#include "Background.h"

namespace ALZ {

	static unsigned int VAO = 0;
	static unsigned int VBO = 0;

	static bool BackgroundBuffersInitilized = false;


	Background::Background()
	{
		if (!BackgroundBuffersInitilized) {

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f),
									 glm::vec2(1.0f, -1.0f),
									 glm::vec2(-1.0f, 1.0f),

									 glm::vec2(1.0f, -1.0f),
									 glm::vec2(1.0f, 1.0f),
									 glm::vec2(-1.0f, 1.0f) };

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

			glBindVertexArray(0);

			BackgroundBuffersInitilized = true;
		}

	}

	void Background::SubmitLight(const RadialLight& light)
	{
		m_RadialLightPositionBuffer[m_RadialLightSubmissionCount] = glm::vec2(light.Position.x, light.Position.y);
		m_RadialLightColorBuffer[m_RadialLightSubmissionCount] = light.Color;
		m_RadialLightConstantBuffer[m_RadialLightSubmissionCount] = light.Constant;
		m_RadialLightLinearBuffer[m_RadialLightSubmissionCount] = light.Linear;
		m_RadialLightQuadraticBuffer[m_RadialLightSubmissionCount] = light.Quadratic;

		m_RadialLightSubmissionCount++;
	}

	void Background::Render(Camera& camera)
	{
		ShaderProgram& BackgroundShader = ShaderProgram::GetBackgroundShader();

		glBindVertexArray(VAO);
		BackgroundShader.Bind();

		//not used light spots
		for (int i = m_RadialLightSubmissionCount; i < MAX_NUM_RADIAL_LIGHTS; i++)
		{
			m_RadialLightPositionBuffer[i] = glm::vec2(10000.0f, 10000.0f);
			m_RadialLightConstantBuffer[i] = 1000.0f;
			m_RadialLightLinearBuffer[i] = 1000.0f;
			m_RadialLightQuadraticBuffer[i] = 1000.0f;
		}
		
		BackgroundShader.SetUniformVec4("baseColor", BaseColor);
		BackgroundShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		BackgroundShader.SetUniformMat4("view", camera.ViewMatrix);

		BackgroundShader.SetUniformVec2s("radialLights.Position", m_RadialLightPositionBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader.SetUniformVec3s("radialLights.Color", m_RadialLightColorBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader.SetUniform1fs("radialLights.Constant", m_RadialLightConstantBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader.SetUniform1fs("radialLights.Linear", m_RadialLightLinearBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader.SetUniform1fs("radialLights.Quadratic", m_RadialLightQuadraticBuffer, MAX_NUM_RADIAL_LIGHTS);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5000.0f));
		BackgroundShader.SetUniformMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		BackgroundShader.Unbind();

		m_RadialLightSubmissionCount = 0;
	}
}