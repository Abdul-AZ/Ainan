#include "ParticleSystem.h"

static unsigned int VBO;
static unsigned int VAO;

ParticleSystem::ParticleSystem()
{
	m_Shader.Init("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag");
	m_Particles.reserve(300);

	for (size_t i = 0; i < 300; i++)
	{
		Particle particle;
		particle.isActive = false;
		m_Particles.push_back(particle);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	const int VertexCountPerCicle = 30;

	glm::vec2 vertices[VertexCountPerCicle];

	vertices[0].x = 0.0f;
	vertices[0].y = 0.0f;
	float degreesBetweenVertices = 360.0f / (VertexCountPerCicle - 6);

	const float PI = 3.1415f;

	for (size_t i = 1; i < VertexCountPerCicle; i++)
	{
		float angle = i * degreesBetweenVertices;
		vertices[i].x = cos(angle * (PI / 180.0));
		vertices[i].y = sin(angle * (PI / 180.0));
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * VertexCountPerCicle, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleSystem::Update(const float& deltaTime)
{
	glm::vec2& size = Window::GetSize();
	glm::mat4 projection = glm::ortho(0.0f, size.x, size.y, 0.0f);
	m_Shader.setUniformMat4("projection", projection);

	for (Particle& particle : m_Particles)
		particle.Update(deltaTime);
}

void ParticleSystem::Draw()
{
	glBindVertexArray(VAO);
	m_Shader.Bind();

	int drawCount = m_Particles.size() / 40;

	for (int i = 0; i < drawCount; i++)
	{
		for (unsigned int j = i * 40; j < 40 + i * 40; j++)
		{
			std::stringstream ss;
			std::string index;
			int i_index = j - i * 40;
			ss << i_index;
			index = ss.str();

			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(m_Particles[j].m_Position.x, m_Particles[j].m_Position.y, 0.0f));
			float t = (m_Particles[j].m_LifeTime - m_Particles[j].m_RemainingLifeTime) / m_Particles[j].m_LifeTime;
			float scale = m_Particles[j].m_ScaleInterpolator.Interpolate(t);
			model = glm::scale(model, glm::vec3(scale, scale, scale));

			if (m_Particles[j].isActive) {
				m_Shader.setUniformMat4(("model[" + index + "]").c_str(), model);
				m_Shader.setUniformVec4(("colorArr[" + index + "]").c_str(), m_Particles[j].m_Color);
			}
			else
				m_Shader.setUniformMat4(("model[" + index + "]").c_str(), glm::mat4(1.0f));

		}
		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, 40);
	}

	int remaining = m_Particles.size() % 40;

	for (unsigned int i = drawCount * 40; i < drawCount * 40 + remaining; i++)
	{
		std::stringstream ss;
		std::string index;
		ss << i;
		index = ss.str();

		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));

		float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;
		float scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);
		model = glm::scale(model, glm::vec3(scale, scale, scale));

		if (m_Particles[i].isActive) {
			m_Shader.setUniformMat4(("model[" + index + "]").c_str(), model);
			m_Shader.setUniformVec4(("colorArr[" + index + "]").c_str(), m_Particles[i].m_Color);
		}
		else
			m_Shader.setUniformMat4(("model[" + index + "]").c_str(), glm::mat4(1.0f));
	}
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, remaining);
}

void ParticleSystem::SpawnParticle(const Particle & particle)
{
	for (Particle& m_particle : m_Particles)
	{
		if (!m_particle.isActive)
		{
			m_particle.m_Position = particle.m_Position;
			m_particle.m_Color = particle.m_Color;
			m_particle.m_Velocity = particle.m_Velocity;
			m_particle.isActive = true;
			m_particle.m_ScaleInterpolator = particle.m_ScaleInterpolator;
			m_particle.SetLifeTime(particle.m_LifeTime);
			break;
		}
	}
}

void ParticleSystem::ClearParticles()
{
	for (Particle& m_particle : m_Particles)
		m_particle.isActive = false;
}
