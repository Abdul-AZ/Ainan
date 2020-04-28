#include <pch.h>

#include "Background.h"

namespace Ainan {

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
	}

	void Background::SubmitLight(const RadialLight& light)
	{
		m_RadialLightPositionBuffer[m_RadialLightSubmissionCount] = light.Position * c_GlobalScaleFactor;
		m_RadialLightColorBuffer[m_RadialLightSubmissionCount] = light.Color;
		m_RadialLightIntensityBuffer[m_RadialLightSubmissionCount] = light.Intensity;

		m_RadialLightSubmissionCount++;
	}

	void Background::SubmitLight(const SpotLight& light)
	{
		m_SpotLightPositionBuffer[m_SpotLightSubmissionCount] = light.Position * c_GlobalScaleFactor;
		m_SpotLightColorBuffer[m_SpotLightSubmissionCount] = light.Color;
		m_SpotLightAngleBuffer[m_SpotLightSubmissionCount] = light.Angle * PI / 180.0f;            //convert to radians
		m_SpotLightInnerCutoffBuffer[m_SpotLightSubmissionCount] = light.InnerCutoff * PI / 180.0f;//convert to radians
		m_SpotLightOuterCutoffBuffer[m_SpotLightSubmissionCount] = light.OuterCutoff * PI / 180.0f;//convert to radians
		m_SpotLightIntensityBuffer[m_SpotLightSubmissionCount] = light.Intensity;

		m_SpotLightSubmissionCount++;
	}

	void Background::DisplayGUI(Environment& env)
	{
		if (!SettingsWindowOpen)
			return;

		ImGui::Begin("Background", &SettingsWindowOpen);
		ImGui::Text("Background\nColor");
		ImGui::SameLine();
		ImGui::ColorEdit3("##Base Background Color", &env.BackgroundColor.r);

		ImGui::Text("Base Light");
		ImGui::SameLine();
		float xPos = ImGui::GetCursorPosX();
		ImGui::SliderFloat("##Base Light", &env.BackgroundBaseLight, 0.0f, 1.0f);

		ImGui::Text("Constant");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Constant", &env.BackgroundConstant, 0.01f);

		ImGui::Text("Linear");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Linear", &env.BackgroundLinear, 0.0001f);

		ImGui::Text("Quadratic");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Quadratic", &env.BackgroundQuadratic, 0.00001f);

		ImGui::End();
	}

	void Background::Draw(Environment& env)
	{
		VAO->Unbind();
		auto& shader = Renderer::ShaderLibrary["BackgroundShader"];
		shader->Bind();

		//not used light spots
		for (int i = m_RadialLightSubmissionCount; i < MAX_NUM_RADIAL_LIGHTS; i++)
			m_RadialLightIntensityBuffer[i] = 0.0f;

		for (int i = m_SpotLightSubmissionCount; i < MAX_NUM_SPOT_LIGHTS; i++)
			m_SpotLightIntensityBuffer[i] = 0.0f;
		
		shader->SetUniformVec3("u_BaseColor", env.BackgroundColor);
		shader->SetUniform1f("u_BaseLight", env.BackgroundBaseLight);
		shader->SetUniform1f("u_Constant", env.BackgroundConstant);
		shader->SetUniform1f("u_Linear", env.BackgroundLinear);
		shader->SetUniform1f("u_Quadratic", env.BackgroundQuadratic);

		//radial light data
		shader->SetUniformVec2s("u_RadialLights.Position", m_RadialLightPositionBuffer, MAX_NUM_RADIAL_LIGHTS);
		shader->SetUniformVec3s("u_RadialLights.Color", m_RadialLightColorBuffer, MAX_NUM_RADIAL_LIGHTS);
		shader->SetUniform1fs("u_RadialLights.Intensity", m_RadialLightIntensityBuffer, MAX_NUM_RADIAL_LIGHTS);

		//spot light data
		shader->SetUniformVec2s("u_SpotLights.Position", m_SpotLightPositionBuffer, MAX_NUM_SPOT_LIGHTS);
		shader->SetUniformVec3s("u_SpotLights.Color", m_SpotLightColorBuffer, MAX_NUM_SPOT_LIGHTS);
		shader->SetUniform1fs("u_SpotLights.Angle", m_SpotLightAngleBuffer, MAX_NUM_SPOT_LIGHTS);
		shader->SetUniform1fs("u_SpotLights.InnerCutoff", m_SpotLightInnerCutoffBuffer, MAX_NUM_SPOT_LIGHTS);
		shader->SetUniform1fs("u_SpotLights.OuterCutoff", m_SpotLightOuterCutoffBuffer, MAX_NUM_SPOT_LIGHTS);
		shader->SetUniform1fs("u_SpotLights.Intensity", m_SpotLightIntensityBuffer, MAX_NUM_SPOT_LIGHTS);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5000.0f));
		shader->SetUniformMat4("u_Model", model);

		Renderer::Draw(*VAO, *shader, Primitive::Triangles, 6);

		VAO->Unbind();
		shader->Unbind();

		m_RadialLightSubmissionCount = 0;
		m_SpotLightSubmissionCount = 0;
	}
}