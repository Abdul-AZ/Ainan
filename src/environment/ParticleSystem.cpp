#include <pch.h>
#include "ParticleSystem.h"

namespace ALZ {

	static unsigned int VBO;
	static unsigned int VAO;
	static bool InitilizedCircleVertices = false;

	static int nameIndextemp = 0;
	ParticleSystem::ParticleSystem() :
		m_EditorOpen(false),
		m_RenameTextOpen(false)
	{
		m_Name = "Particle System (" + std::to_string(nameIndextemp) + ")";
		m_ID = nameIndextemp;
		nameIndextemp++;

		m_Noise.Init();

		//TODO pass as a parameter
		m_ParticleCount = 1000;
		m_ParticleInfoBuffer = malloc((sizeof(glm::mat4) + sizeof(glm::vec4)) * m_ParticleCount);
		memset(m_ParticleInfoBuffer, 0, (sizeof(glm::mat4) + sizeof(glm::vec4)) * m_ParticleCount);

		m_Particles.reserve(m_ParticleCount);
		for (size_t i = 0; i < m_ParticleCount; i++)
		{
			Particle particle;
			particle.isActive = false;
			m_Particles.push_back(particle);
		}

		//initilize the vertices only the first time a particle system is created
		if (!InitilizedCircleVertices) {

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

			InitilizedCircleVertices = true;
		}
	}

	void ParticleSystem::Update(const float& deltaTime, Camera& camera)
	{
		ShaderProgram& CircleShader = ShaderProgram::GetCircleInstancedShader();

		CircleShader.SetUniformMat4("projection", camera.GetProjectionMatrix());
		CircleShader.SetUniformMat4("view", camera.GetViewMatrix());

		if (m_Customizer.Mode == SpawnMode::SpawnOnPoint || m_Customizer.Mode == SpawnMode::SpawnOnLine || m_Customizer.Mode == SpawnMode::SpawnOnCircle ||(m_Customizer.Mode == SpawnMode::SpawnOnMousePosition && m_ShouldSpawnParticles)) {
			SpawnAllParticlesOnQue(deltaTime, camera);
		}

		m_ActiveParticleCount = 0;
		for (Particle& particle : m_Particles) {

			if (m_Customizer.m_NoiseCustomizer.m_NoiseEnabled) {
				particle.m_Velocity.x += m_Noise.Noise(particle.m_Position.x, particle.m_Position.y) * m_Customizer.m_NoiseCustomizer.m_NoiseStrength;
				particle.m_Velocity.y += m_Noise.Noise(particle.m_Position.x + 30, particle.m_Position.y - 30) * m_Customizer.m_NoiseCustomizer.m_NoiseStrength;
			}

			particle.Update(deltaTime);

			//update active particle count
			if (particle.isActive)
				m_ActiveParticleCount++;
		}
	}

	void ParticleSystem::Draw()
	{
		glBindVertexArray(VAO);
		ShaderProgram& CircleShader = ShaderProgram::GetCircleInstancedShader();
		CircleShader.Bind();

		glm::mat4* modelBuffer = (glm::mat4*) m_ParticleInfoBuffer;
		glm::vec4* colorBuffer = (glm::vec4*) ((char*)m_ParticleInfoBuffer + m_ParticleCount * sizeof(glm::mat4));

		for (int i = 0; i < m_ParticleCount; i++)
		{
			if (m_Particles[i].m_LifeTime == 0.0f)
				m_Particles[i].m_LifeTime = 0.001f;
			float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;
			glm::mat4 model = glm::mat4(1.0f);

			if (m_Particles[i].isActive) {

				model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));
				float scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);
				model = glm::scale(model, glm::vec3(scale, scale, scale));

			}
			else
			{
				model = glm::translate(model, glm::vec3(-10000, -10000, 0.0f));
			}

			modelBuffer[i] = model;

			colorBuffer[i] = m_Particles[i].m_Color.Interpolate(t);
		}


		int drawCount = m_Particles.size() / 40;

		for (int i = 0; i < drawCount; i++)
		{
			CircleShader.SetUniformVec4s("colorArr", &colorBuffer[i * 40], 40);
			CircleShader.SetUniformMat4s("model", &modelBuffer[i * 40], 40);
			glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 26, 40);
		}

		int remaining = m_Particles.size() % 40;

		CircleShader.SetUniformVec4s("colorArr", &colorBuffer[drawCount * 40], remaining);
		CircleShader.SetUniformMat4s("model", &modelBuffer[drawCount * 40], remaining);
		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 26, remaining);
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

	ParticleSystem::ParticleSystem(const ParticleSystem& Psystem) :
		m_Customizer(Psystem.m_Customizer)
	{
		m_ParticleInfoBuffer = malloc((sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);
		memcpy(m_ParticleInfoBuffer, Psystem.m_ParticleInfoBuffer, (sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);

		m_Particles = Psystem.m_Particles;
		m_ParticleCount = Psystem.m_ParticleCount;
		m_Name = Psystem.m_Name;
		m_EditorOpen = Psystem.m_EditorOpen;
		m_ID = Psystem.m_ID;
		m_RenameTextOpen = Psystem.m_RenameTextOpen;
		m_Noise.Init();
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGUI(Camera& camera)
	{
		if (m_EditorOpen)
			m_Customizer.DisplayGUI(m_Name, m_EditorOpen);

		//update editor line 
		if (m_Customizer.Mode == SpawnMode::SpawnOnLine && m_Selected)
		{
			glm::vec2 pointDispositionFromCenter = m_Customizer.m_LineLength * glm::vec2(cos(m_Customizer.m_LineAngle * 3.14159265 / 180.0f), sin(m_Customizer.m_LineAngle * 3.14159265 / 180.0f));

			glm::vec2 startLinePoint = m_Customizer.m_LinePosition + pointDispositionFromCenter;
			glm::vec2 endLinePoint = m_Customizer.m_LinePosition - pointDispositionFromCenter;

			m_Customizer.m_Line.SetPoints(startLinePoint * (float)1000, endLinePoint * (float)1000);
			m_Customizer.m_Line.Render(camera);
		}
		else if (m_Customizer.Mode == SpawnMode::SpawnOnCircle && m_Selected) 
		{
			m_Customizer.m_CircleOutline.Render(camera);
		}
	}

	void ParticleSystem::SpawnAllParticlesOnQue(const float& deltaTime, Camera& camera)
	{
		m_TimeTillNextParticleSpawn -= deltaTime;
		if (m_TimeTillNextParticleSpawn < 0.0f) {
			m_TimeTillNextParticleSpawn = abs(m_TimeTillNextParticleSpawn);

			while (m_TimeTillNextParticleSpawn > 0.0f) {
				Particle p = m_Customizer.GetParticle();
				if (m_Customizer.Mode == SpawnMode::SpawnOnMousePosition)
					p.m_Position -= glm::vec2(camera.Position.x, camera.Position.y);
				SpawnParticle(p);
				m_TimeTillNextParticleSpawn -= m_Customizer.GetTimeBetweenParticles();
			}

			m_TimeTillNextParticleSpawn = m_Customizer.GetTimeBetweenParticles();
		}
	}

	ParticleSystem::~ParticleSystem()
	{
		free(m_ParticleInfoBuffer);
	}
}