#include "ParticleCustomizer.h"

ParticleCustomizer::ParticleCustomizer() :
	mt(std::random_device{}())
{

}

void ParticleCustomizer::DisplayGUI(const std::string& windowName)
{
	ImGui::Begin(windowName.c_str());

	m_VelocityCustomizer.DisplayGUI();
	m_ColorCustomizer.DisplayGUI();

	m_LifetimeCustomizer.DisplayGUI();

	m_ScaleCustomizer.DisplayGUI();

	ImGui::End();
}

void ParticleCustomizer::Update()
{
	double xpos, ypos;
	glfwGetCursorPos(&Window::GetWindow(), &xpos, &ypos);


	m_Particle.m_Position = glm::vec2(xpos, ypos);
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
