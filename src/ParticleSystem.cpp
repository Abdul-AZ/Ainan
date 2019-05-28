#include <pch.h>
#include "ParticleSystem.h"

namespace ALZ {

	static unsigned int VBO;
	static unsigned int VAO;
	static bool InitilizedCircleVertices = false;

	static int nameIndextemp = 0;

	Texture DefaultTexture;

	ParticleSystem::ParticleSystem()
	{
		Type = InspectorObjectType::ParticleSystemType;

		m_Name = "Particle System (" + std::to_string(nameIndextemp) + ")";
		ID = nameIndextemp;
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

			std::cout << glGetError() << std::endl;

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			//						 Position				  Texture Coordinates
			glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0),
									 glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 1.0),
									 glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 0.0),

									 glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 0.0),
									 glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),
									 glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 1.0) };

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);


			DefaultTexture.Init("res/Circle.png", 4);
			glBindTexture(GL_TEXTURE_2D, (GLuint)DefaultTexture.TextureID);

			std::cout << glGetError() << std::endl;
			InitilizedCircleVertices = true;
		}
	}

	void ParticleSystem::Update(const float& deltaTime)
	{
		if (Customizer.Mode == SpawnMode::SpawnOnPoint || Customizer.Mode == SpawnMode::SpawnOnLine || Customizer.Mode == SpawnMode::SpawnOnCircle) {
			SpawnAllParticlesOnQue(deltaTime);
		}

		ActiveParticleCount = 0;
		for (Particle& particle : m_Particles) {

			if (Customizer.m_NoiseCustomizer.m_NoiseEnabled) {
				particle.m_Velocity.x += m_Noise.Noise(particle.m_Position.x, particle.m_Position.y) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
				particle.m_Velocity.y += m_Noise.Noise(particle.m_Position.x + 30, particle.m_Position.y - 30) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
			}

			particle.Update(deltaTime);

			//update active particle count
			if (particle.isActive)
				ActiveParticleCount++;
		}
	}

	void ParticleSystem::UpdateUniforms(Camera & camera)
	{
		ShaderProgram& CircleShader = ShaderProgram::GetCircleInstancedShader();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		CircleShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		CircleShader.SetUniformMat4("view", camera.ViewMatrix);
	}

	void ParticleSystem::Render(Camera& camera)
	{
		glBindVertexArray(VAO);
		ShaderProgram& CircleShader = ShaderProgram::GetCircleInstancedShader();
		CircleShader.Bind();
		CircleShader.SetUniform1i("particleTexture", 0);
		if (Customizer.m_TextureCustomizer.UseDefaultTexture)
			DefaultTexture.Bind(0);
		else
			Customizer.m_TextureCustomizer.ParticleTexture.Bind(0);

		glm::mat4* modelBuffer = (glm::mat4*) m_ParticleInfoBuffer;
		glm::vec4* colorBuffer = (glm::vec4*) ((char*)m_ParticleInfoBuffer + m_ParticleCount * sizeof(glm::mat4));

		for (unsigned int i = 0; i < m_ParticleCount; i++)
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


		int drawCount = (int)m_Particles.size() / 40;

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
		for (unsigned int i = 0; i < m_ParticleCount; i++)
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
		Customizer(Psystem.Customizer)
	{
		m_ParticleInfoBuffer = malloc((sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);
		memcpy(m_ParticleInfoBuffer, Psystem.m_ParticleInfoBuffer, (sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);

		m_Particles = Psystem.m_Particles;
		m_ParticleCount = Psystem.m_ParticleCount;
		m_Name = Psystem.m_Name;
		EditorOpen = Psystem.EditorOpen;
		ID = Psystem.ID;
		RenameTextOpen = Psystem.RenameTextOpen;
		m_Noise.Init();
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGUI(Camera& camera)
	{
		if (EditorOpen)
			Customizer.DisplayGUI(m_Name, EditorOpen);

		//update editor line 
		if (Customizer.Mode == SpawnMode::SpawnOnLine && Selected)
		{
			glm::vec2 pointDispositionFromCenter = Customizer.m_LineLength * glm::vec2(cos(Customizer.m_LineAngle * 3.14159265 / 180.0f), sin(Customizer.m_LineAngle * 3.14159265 / 180.0f));

			glm::vec2 startLinePoint = Customizer.m_LinePosition + pointDispositionFromCenter;
			glm::vec2 endLinePoint = Customizer.m_LinePosition - pointDispositionFromCenter;

			Customizer.m_Line.SetPoints(startLinePoint, endLinePoint);
			Customizer.m_Line.Render(camera);
		}
		else if (Customizer.Mode == SpawnMode::SpawnOnCircle && Selected) 
		{
			Customizer.m_CircleOutline.Render(camera);
		}
	}

	void ParticleSystem::SpawnAllParticlesOnQue(const float& deltaTime)
	{
		TimeTillNextParticleSpawn -= deltaTime;
		if (TimeTillNextParticleSpawn < 0.0f) {
			TimeTillNextParticleSpawn = abs(TimeTillNextParticleSpawn);

			while (TimeTillNextParticleSpawn > 0.0f) {
				Particle p = Customizer.GetParticle();
				SpawnParticle(p);
				TimeTillNextParticleSpawn -= Customizer.GetTimeBetweenParticles();
			}

			TimeTillNextParticleSpawn = Customizer.GetTimeBetweenParticles();
		}
	}

	ParticleSystem::~ParticleSystem()
	{
		free(m_ParticleInfoBuffer);
	}
}