#include "ParticleSystem.h"

static unsigned int VBO;
static unsigned int VAO;

ParticleSystem::ParticleSystem()
{
	m_Shader.Init("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag");

	//TODO pass as a parameter
	m_ParticleCount = 600;
	m_ParticleInfoBuffer = malloc((sizeof(glm::mat4) + sizeof(glm::vec4)) * m_ParticleCount);
	memset(m_ParticleInfoBuffer, 0, (sizeof(glm::mat4) + sizeof(glm::vec4)) * m_ParticleCount);

	m_Particles.reserve(m_ParticleCount);
	for (size_t i = 0; i < m_ParticleCount; i++)
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

	glm::mat4* modelBuffer = (glm::mat4*) m_ParticleInfoBuffer;
	glm::vec4* colorBuffer = (glm::vec4*) ((char*)m_ParticleInfoBuffer + m_ParticleCount * sizeof(glm::mat4));

	for (int i = 0; i < m_ParticleCount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);

		if (m_Particles[i].isActive) {

			model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));
			float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;
			float scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);
			model = glm::scale(model, glm::vec3(scale, scale, scale));

		}
		else
		{
			model = glm::translate(model, glm::vec3(-10000, -10000, 0.0f));
		}

		modelBuffer[i] = model;

		colorBuffer[i] = m_Particles[i].m_Color;
	}


	int drawCount = m_Particles.size() / 40;

	for (int i = 0; i < drawCount; i++)
	{
		m_Shader.setUniformVec4s("colorArr", &colorBuffer[i * 40], 40);
		m_Shader.setUniformMat4s("model", &modelBuffer[i * 40], 40);
		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, 40);
	}

	int remaining = m_Particles.size() % 40;

	m_Shader.setUniformVec4s("colorArr", &colorBuffer[drawCount * 40], remaining);
	m_Shader.setUniformMat4s("model", &modelBuffer[drawCount * 40], remaining);
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, remaining);
}

void ParticleSystem::SpawnParticle(const Particle & particle)
{
	for (int i = 0; i < m_ParticleCount; i++)
	{
		if (!m_Particles[i].isActive)
		{
			m_Particles[i].m_Position = particle.m_Position;
			m_Particles[i].m_Color = particle.m_Color;
			m_Particles[i].m_Velocity = particle.m_Velocity;
			m_Particles[i].isActive = true;
			m_Particles[i].m_ScaleInterpolator = particle.m_ScaleInterpolator;
			m_Particles[i].SetLifeTime(particle.m_LifeTime);
			break;
		}
	}
}

void ParticleSystem::ClearParticles()
{
	for (Particle& m_particle : m_Particles)
		m_particle.isActive = false;
}

ParticleSystem::~ParticleSystem()
{
	free(m_ParticleInfoBuffer);
}
