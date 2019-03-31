#include <pch.h>
#include "ParticleCustomizer.h"

ParticleCustomizer::ParticleCustomizer() :
	mt(std::random_device{}())
{}

static std::string GetAsText(const SpawnMode& mode) 
{
	switch (mode)
	{
	case SpawnMode::SpawnOnMousePosition:
		return "Spawn On Mouse";

	case SpawnMode::SpawnOnPosition:
		return "Spawn On Specified Position";

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
			bool is_active = m_Mode == SpawnMode::SpawnOnPosition;
			if (ImGui::Selectable(GetAsText(SpawnMode::SpawnOnPosition).c_str(), &is_active)) {
				ImGui::SetItemDefaultFocus();
				m_Mode = SpawnMode::SpawnOnPosition;
			}
		}

		ImGui::EndCombo();
	}

	if (ImGui::TreeNode("Emission")) {

		ImGui::DragFloat("Particles Per Second", &m_ParticlesPerSecond, 1.0f, 0.1f, 1000.0f);

		ImGui::TreePop();
	}

	if (m_Mode == SpawnMode::SpawnOnPosition) {
		if (ImGui::TreeNode("Position")) {

			ImGui::DragFloat2("Starting Position :", &m_SpawnPosition.x, 1.0f, 0.0f, 1000.0f);

			ImGui::TreePop();
		}
	}

	m_VelocityCustomizer.DisplayGUI();
	m_ColorCustomizer.DisplayGUI();
	m_LifetimeCustomizer.DisplayGUI();
	m_ScaleCustomizer.DisplayGUI();

	ImGui::End();
}

void ParticleCustomizer::Update()
{
	if (m_Mode == SpawnMode::SpawnOnMousePosition) {
		double xpos, ypos;
		glfwGetCursorPos(&Window::GetWindow(), &xpos, &ypos);
		m_Particle.m_Position = glm::vec2(xpos, ypos);
	} 
	else if (m_Mode == SpawnMode::SpawnOnPosition) {
		glm::vec2& size = Window::GetSize();

		glm::vec2 spawnPosition = { (m_SpawnPosition.x / 1000.0f) * size.x, size.y - (m_SpawnPosition.y / 1000.0f) * size.y };

		m_Particle.m_Position = spawnPosition;
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
