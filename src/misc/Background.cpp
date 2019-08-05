#include <pch.h>

#include "Background.h"

namespace ALZ {

	Background::Background()
	{
		VAO = Renderer::CreateVertexArray();
		VAO->Bind();

		glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f),
								 glm::vec2(1.0f, -1.0f),
								 glm::vec2(-1.0f, 1.0f),

								 glm::vec2(1.0f, -1.0f),
								 glm::vec2(1.0f, 1.0f),
								 glm::vec2(-1.0f, 1.0f) };

		VBO = Renderer::CreateVertexBuffer(vertices, sizeof(vertices));
		VBO->SetLayout({ ShaderVariableType::Vec2 });

		VAO->Unbind();

		BackgroundShader = Renderer::CreateShaderProgram("shaders/Background.vert", "shaders/Background.frag");
	}

	void Background::SubmitLight(const RadialLight& light)
	{
		m_RadialLightPositionBuffer[m_RadialLightSubmissionCount] = light.Position * GlobalScaleFactor;
		m_RadialLightColorBuffer[m_RadialLightSubmissionCount] = light.Color;
		m_RadialLightIntensityBuffer[m_RadialLightSubmissionCount] = light.Intensity;

		m_RadialLightSubmissionCount++;
	}

	void Background::SubmitLight(const SpotLight& light)
	{
		m_SpotLightPositionBuffer[m_SpotLightSubmissionCount] = light.Position * GlobalScaleFactor;
		m_SpotLightColorBuffer[m_SpotLightSubmissionCount] = light.Color;
		m_SpotLightAngleBuffer[m_SpotLightSubmissionCount] = light.Angle * PI / 180.0f;            //convert to radians
		m_SpotLightInnerCutoffBuffer[m_SpotLightSubmissionCount] = light.InnerCutoff * PI / 180.0f;//convert to radians
		m_SpotLightOuterCutoffBuffer[m_SpotLightSubmissionCount] = light.OuterCutoff * PI / 180.0f;//convert to radians
		m_SpotLightIntensityBuffer[m_SpotLightSubmissionCount] = light.Intensity;

		m_SpotLightSubmissionCount++;
	}

	void Background::DisplayGUI()
	{
		if (!SettingsWindowOpen)
			return;

		ImGui::Begin("Background", &SettingsWindowOpen);
		ImGui::ColorEdit3("Base Background Color", &BaseColor.r);
		ImGui::SliderFloat("Base Light", &BaseLight, 0.0f, 1.0f);

		ImGui::DragFloat("Constant", &Constant, 0.01f);
		ImGui::DragFloat("Linear", &Linear, 0.0001f);
		ImGui::DragFloat("Quadratic", &Quadratic, 0.00001f);

		ImGui::End();
	}

	void Background::Draw()
	{
		VAO->Unbind();
		BackgroundShader->Bind();

		//not used light spots
		for (int i = m_RadialLightSubmissionCount; i < MAX_NUM_RADIAL_LIGHTS; i++)
			m_RadialLightIntensityBuffer[i] = 0.0f;

		for (int i = m_SpotLightSubmissionCount; i < MAX_NUM_SPOT_LIGHTS; i++)
			m_SpotLightIntensityBuffer[i] = 0.0f;
		
		BackgroundShader->SetUniformVec3("u_BaseColor", BaseColor);
		BackgroundShader->SetUniform1f("u_BaseLight", BaseLight);
		BackgroundShader->SetUniform1f("u_Constant", Constant);
		BackgroundShader->SetUniform1f("u_Linear", Linear);
		BackgroundShader->SetUniform1f("u_Quadratic", Quadratic);

		//radial light data
		BackgroundShader->SetUniformVec2s("u_RadialLights.Position", m_RadialLightPositionBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader->SetUniformVec3s("u_RadialLights.Color", m_RadialLightColorBuffer, MAX_NUM_RADIAL_LIGHTS);
		BackgroundShader->SetUniform1fs("u_RadialLights.Intensity", m_RadialLightIntensityBuffer, MAX_NUM_RADIAL_LIGHTS);

		//spot light data
		BackgroundShader->SetUniformVec2s("u_SpotLights.Position", m_SpotLightPositionBuffer, MAX_NUM_SPOT_LIGHTS);
		BackgroundShader->SetUniformVec3s("u_SpotLights.Color", m_SpotLightColorBuffer, MAX_NUM_SPOT_LIGHTS);
		BackgroundShader->SetUniform1fs("u_SpotLights.Angle", m_SpotLightAngleBuffer, MAX_NUM_SPOT_LIGHTS);
		BackgroundShader->SetUniform1fs("u_SpotLights.InnerCutoff", m_SpotLightInnerCutoffBuffer, MAX_NUM_SPOT_LIGHTS);
		BackgroundShader->SetUniform1fs("u_SpotLights.OuterCutoff", m_SpotLightOuterCutoffBuffer, MAX_NUM_SPOT_LIGHTS);
		BackgroundShader->SetUniform1fs("u_SpotLights.Intensity", m_SpotLightIntensityBuffer, MAX_NUM_SPOT_LIGHTS);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5000.0f));
		BackgroundShader->SetUniformMat4("u_Model", model);

		Renderer::Draw(*VAO, *BackgroundShader, Primitive::Triangles, 6);

		VAO->Unbind();
		BackgroundShader->Unbind();

		m_RadialLightSubmissionCount = 0;
		m_SpotLightSubmissionCount = 0;
	}
}