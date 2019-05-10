#include <pch.h>
#include "ParticleCustomizer.h"

namespace ALZ {

	ParticleCustomizer::ParticleCustomizer() :
		mt(std::random_device{}())
	{
		m_Line.Color = glm::vec4(0.0f, 0.7f, 0.0f, 0.85f);
		m_CircleOutline.Color = glm::vec4(0.0f, 0.7f, 0.0f, 0.85f);
		m_CircleOutline.Position = { 0.5f, 0.5f };
	}

	static std::string GetAsText(const SpawnMode& mode)
	{
		switch (mode)
		{
		case SpawnMode::SpawnOnPoint:
			return "Spawn On Point";

		case SpawnMode::SpawnOnLine:
			return "Spawn On Line";

		case SpawnMode::SpawnOnCircle:
			return "Spawn On Circle";

		default:
			return "";
		}
	}

	void ParticleCustomizer::DisplayGUI(const std::string& windowName, bool& windowOpen)
	{
		ImGui::Begin((windowName.size() > 0) ? windowName.c_str() : "No Name", &windowOpen);

		if (ImGui::BeginCombo("Spawn Mode", GetAsText(Mode).c_str())) {

			{
				bool is_active = Mode == SpawnMode::SpawnOnPoint;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnPoint).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnPoint;
				}
			}

			{
				bool is_active = Mode == SpawnMode::SpawnOnLine;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnLine).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnLine;
				}
			}

			{
				bool is_active = Mode == SpawnMode::SpawnOnCircle;
				if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnCircle).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnCircle;
				}
			}

			ImGui::EndCombo();
		}

		m_TextureCustomizer.DisplayGUI();

		if (ImGui::TreeNode("Emission")) {

			ImGui::DragFloat("Particles Per Second", &m_ParticlesPerSecond, 1.0f, 0.1f, 1000.0f);

			ImGui::TreePop();
		}

		if (Mode == SpawnMode::SpawnOnPoint) {
			if (ImGui::TreeNode("Position")) {

				ImGui::DragFloat2("Starting Position :", &m_SpawnPosition.x, 0.001f);

				ImGui::TreePop();
			}
		}
		else if (Mode == SpawnMode::SpawnOnLine)
		{
			if (ImGui::TreeNode("Position")) {

				ImGui::DragFloat2("Line Position :", &m_LinePosition.x, 0.001f);
				ImGui::DragFloat("Line Length :", &m_LineLength, 0.001f);
				ImGui::DragFloat("Line Rotation :", &m_LineAngle, 1.0f, 0.0f, 360.0f);

				ImGui::TreePop();
			}
		}
		else if (Mode == SpawnMode::SpawnOnCircle)
		{
			if (ImGui::TreeNode("Position")) {

				ImGui::DragFloat2("Circle Position :", &m_CircleOutline.Position.x, 0.001f);
				ImGui::DragFloat("Circle Radius :", &m_CircleOutline.Radius, 0.001f);

				m_CircleOutline.Radius = std::clamp(m_CircleOutline.Radius, 0.001f, 10000.0f);

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
		switch (Mode)
		{
		case SpawnMode::SpawnOnPoint: {
			glm::vec2 spawnPosition = { m_SpawnPosition.x * GlobalScaleFactor, m_SpawnPosition.y * GlobalScaleFactor };
			m_Particle.m_Position = spawnPosition;
			break;
		}

		case SpawnMode::SpawnOnLine: {
			std::uniform_real_distribution<float> dest(0.0f, 1.0f);
			m_Particle.m_Position = m_Line.GetPointInLine(dest(mt));
			break;
		}

		case SpawnMode::SpawnOnCircle: {
			//random angle between 0 and 2pi (360 degrees)
			std::uniform_real_distribution<float> dest(0.0f, 2 * 3.14159);
			m_Particle.m_Position = m_CircleOutline.GetPointByAngle(dest(mt));
			break;
		}
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