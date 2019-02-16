#include "ParticleSystem.h"

static unsigned int VBO;
static unsigned int VAO;

ParticleSystem::ParticleSystem() :
	Scale(10)
{
	m_Shader.Init("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag");
	m_Particles.reserve(100);

	for (size_t i = 0; i < 100; i++)
	{
		Particle particle(glm::vec2(0, 0),glm::vec4(1.0f,1.0f,1.0f,1.0f));
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

	for (unsigned int i = 0; i < 100; i++)
	{
		std::stringstream ss;
		std::string index;
		ss << i;
		index = ss.str();

		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));
		model = glm::scale(model, glm::vec3(m_Particles[i].m_Scale, m_Particles[i].m_Scale, m_Particles[i].m_Scale));

		if(m_Particles[i].isActive)
			m_Shader.setUniformMat4(("model[" + index + "]").c_str(), model);
		else
			m_Shader.setUniformMat4(("model[" + index + "]").c_str(), glm::mat4(1.0f));
	}
}

void ParticleSystem::Draw()
{
	glBindVertexArray(VAO);
	m_Shader.Bind();
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, 100);
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
			m_particle.m_Scale = particle.m_Scale;
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
