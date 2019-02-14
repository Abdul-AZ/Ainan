#include "ParticleSystem.h"

static unsigned int VBO;
static unsigned int VAO;

ParticleSystem::ParticleSystem() 
{
	m_Shader.Init("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag");
	m_Particles.reserve(100);

	for (size_t i = 0; i < 100; i++)
	{
		Particle particle(glm::vec2(0.0f,0.0f),glm::vec4(1.0f,1.0f,1.0f,1.0f));
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

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(1000), static_cast<GLfloat>(1000 * 9 / 16), 0.0f);
	m_Shader.setUniformMat4("projection", projection);

	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2, 2, 2));
	m_Shader.setUniformMat4("model", model);

	for (unsigned int i = 0; i < 100; i++)
	{
		std::stringstream ss;
		std::string index;
		ss << i;
		index = ss.str();
		m_Shader.setUniformVec2(("positions[" + index + "]").c_str(), glm::vec2(i * 3,i * 3));
	}
}

void ParticleSystem::Update()
{
	for (Particle& particle : m_Particles)
		particle.Update();
}

void ParticleSystem::Draw()
{
	glBindVertexArray(VAO);
	m_Shader.Bind();
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 30, 100);
}
