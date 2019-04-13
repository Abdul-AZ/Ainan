#include <pch.h>
#include "ParticleCustomizer.h"

namespace ALZ {

	ParticleCustomizer::ParticleCustomizer() :
		mt(std::random_device{}())
	{
		m_Line.SetColor(glm::vec4(0.0f, 0.7f, 0.0f, 0.85f));
	}

	static std::string GetAsText(const SpawnMode& mode)
	{
		switch (mode)
		{
		case SpawnMode::SpawnOnMousePosition:
			return "Spawn On Mouse";

		case SpawnMode::SpawnOnPoint:
			return "Spawn On Point";

		case SpawnMode::SpawnOnLine:
			return "Spawn On Line";

		default:
			return "";
		}
	}

	void ParticleCustomizer::DisplayGUI(const std::string& windowName, bool& windowOpen)
	{
		ImGui::Begin((windowName.size() > 0) ? windowName.c_str() : "No Name", &windowOpen);

		if (ImGui::BeginCombo("Spawn Mode", GetAsText(m_Mode).c_str())) {

			{
				bool is_active = m_Mode == SpawnMode::SpawnOnMousePosition;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnMousePosition).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					m_Mode = SpawnMode::SpawnOnMousePosition;
				}
			}

			{
				bool is_active = m_Mode == SpawnMode::SpawnOnPoint;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnPoint).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					m_Mode = SpawnMode::SpawnOnPoint;
				}
			}

			{
				bool is_active = m_Mode == SpawnMode::SpawnOnLine;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnLine).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					m_Mode = SpawnMode::SpawnOnLine;
				}
			}

			ImGui::EndCombo();
		}

		if (ImGui::TreeNode("Emission")) {

			ImGui::DragFloat("Particles Per Second", &m_ParticlesPerSecond, 1.0f, 0.1f, 1000.0f);

			ImGui::TreePop();
		}

		if (m_Mode == SpawnMode::SpawnOnPoint) {
			if (ImGui::TreeNode("Position")) {

				ImGui::DragFloat2("Starting Position :", &m_SpawnPosition.x, 0.001f);

				ImGui::TreePop();
			}
		}

		if (m_Mode == SpawnMode::SpawnOnLine)
		{
			if (ImGui::TreeNode("Position")) {

				ImGui::DragFloat2("Line Position :", &m_LinePosition.x, 0.001f);
				ImGui::DragFloat("Line Length :", &m_LineLength, 0.001f);
				ImGui::DragFloat("Line Rotation :", &m_LineAngle, 1.0f, 0.0f, 360.0f);

				ImGui::TreePop();
			}
		}

		m_NoiseCustomizer.DisplayGUI();
		m_VelocityCustomizer.DisplayGUI();
		m_ColorCustomizer.DisplayGUI();
		m_LifetimeCustomizer.DisplayGUI();
		m_ScaleCustomizer.DisplayGUI();

		ImGui::End();
	}

	void ParticleCustomizer::Update()
	{
		switch (m_Mode)
		{
		case SpawnMode::SpawnOnMousePosition:
			double xpos, ypos;
			glfwGetCursorPos(&Window::GetWindow(), &xpos, &ypos);
			m_Particle.m_Position = glm::vec2(xpos, ypos);
			break;


		case SpawnMode::SpawnOnPoint:
			glm::vec2 spawnPosition = { m_SpawnPosition.x * 1000, 1000 - m_SpawnPosition.y * 1000 };
			m_Particle.m_Position = spawnPosition;
			break;


		case SpawnMode::SpawnOnLine:
			std::uniform_real_distribution<float> dest(0.0f, 1.0f);
			m_Particle.m_Position = m_Line.GetPointInLine(dest(mt));
			break;
		}

		m_Particle.m_Velocity = m_VelocityCustomizer.GetVelocity();
		m_Particle.m_Color = m_ColorCustomizer.GetColorInterpolator();
		m_Particle.SetLifeTime(m_LifetimeCustomizer.GetLifetime());


		m_Particle.m_ScaleInterpolator = m_ScaleCustomizer.GetScaleInterpolator();
	}

	Particle& ParticleCustomizer::GetParticle()
	{
		Update();
		return m_Particle;
	}
}