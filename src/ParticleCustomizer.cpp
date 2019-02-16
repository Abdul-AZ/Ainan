#include "ParticleCustomizer.h"

ParticleCustomizer::ParticleCustomizer() :
	mt(std::random_device{}())
{

	m_Particle.m_Position = { 0, 0 };
	m_Particle.m_Color = { 0, 0, 0, 0 };
	m_Particle.m_Scale = 10;
	m_Particle.SetLifeTime(2);

	RandomVelocity = true;
	minVelocity = { -100, -100 };
	maxVelocity = { 100, 100 };
	defiendVelocity = { 100, -100 };
}

void ParticleCustomizer::DisplayImGuiCustomizer()
{
	ImGui::Begin("Particle Settings");

	if (ImGui::TreeNode("Starting Velocity")) {

		ImGui::Checkbox("Random Between 2 Numbers", &RandomVelocity);

		if (RandomVelocity)
		{
			if (maxVelocity.x < minVelocity.x)
				maxVelocity.x = minVelocity.x;

			if (maxVelocity.y < minVelocity.y)
				maxVelocity.y = minVelocity.y;

			ImGui::SliderFloat2("Minimum Velocity:", &minVelocity.x, -500.0f, 500.0f);
			ImGui::SliderFloat2("Maximum Velocity:", &maxVelocity.x, -500.0f, 500.0f);
		}
		else
		{
			ImGui::SliderFloat2("Velocity:", &defiendVelocity.x, -500.0f, 500.0f);
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void ParticleCustomizer::Update()
{
	double xpos, ypos;
	glfwGetCursorPos(&Window::GetWindow(), &xpos, &ypos);

	m_Particle.m_Position = glm::vec2(xpos, ypos);
	m_Particle.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0);

	if (RandomVelocity) {
		std::uniform_real_distribution<float> dist_velocity_x(minVelocity.x, maxVelocity.x);
		std::uniform_real_distribution<float> dist_velocity_y(minVelocity.y, maxVelocity.y);
		m_Particle.m_Velocity = glm::vec2(dist_velocity_x(mt), dist_velocity_y(mt));
	}
	else {
		m_Particle.m_Velocity = defiendVelocity;
	}

	std::uniform_real_distribution<float> dist_time(0.1f, 3.0f);

	m_Particle.SetLifeTime(dist_time(mt));

	std::uniform_real_distribution<float> dist_scale(8.0f, 15.0f);
	m_Particle.m_Scale = dist_scale(mt);
}

Particle& ParticleCustomizer::GetParticle()
{
	Update();
	return m_Particle;
}
