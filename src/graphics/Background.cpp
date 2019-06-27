#include <pch.h>

#include "Background.h"

namespace ALZ {

	static VertexArray* VAO = nullptr;
	static VertexBuffer* VBO = nullptr;

	static bool BackgroundBuffersInitilized = false;

	Background::Background()
	{
		if (!BackgroundBuffersInitilized) {

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f),
									 glm::vec2(1.0f, -1.0f),
									 glm::vec2(-1.0f, 1.0f),

									 glm::vec2(1.0f, -1.0f),
									 glm::vec2(1.0f, 1.0f),
									 glm::vec2(-1.0f, 1.0f) };

			VBO = Renderer::CreateVertexBuffer(vertices, sizeof(vertices)).release();
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			VAO->Unbind();

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

		VAO->Unbind();
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

		Renderer::Draw(*VAO, BackgroundShader, Primitive::Triangles, 6);

		VAO->Unbind();
		BackgroundShader.Unbind();

		m_RadialLightSubmissionCount = 0;
	}
}