#pragma once

#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderProgram.h"

struct Particle 
{
	Particle(const glm::vec2& position, const glm::vec4& color) :
		m_Position(position),
		m_Color(color)
	{}

	//TODO pass deltaTime here
	void Update() {
		m_Position += m_Velocity;
	}

	glm::vec2 m_Position;
	glm::vec4 m_Color;

	glm::vec2 m_Velocity;
};

class ParticleSystem 
{
public:
	ParticleSystem();

	//TODO pass deltaTime here
	void Update();

	void Draw();

private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
};