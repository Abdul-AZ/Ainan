#include <pch.h>
#include "ParticleSystem.h"

#include "renderer/VertexArray.h"
#include "renderer/VertexBuffer.h"

namespace ALZ {
	static bool InitilizedCircleVertices = false;
	static VertexArray* VAO = nullptr;
	static VertexBuffer* VBO = nullptr;
	static ShaderProgram* CircleInstancedShader = nullptr;

	static int nameIndextemp = 0;

	static Texture* DefaultTexture;

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

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			//						 Position				  Texture Coordinates
			glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0),
									 glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 0.0),
									 glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 1.0),

									 glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 0.0),
									 glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),
									 glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 1.0) };


			VBO = Renderer::CreateVertexBuffer(vertices, sizeof(vertices)).release();

			//				 Position					Texture Coordinates
			VBO->SetLayout({ ShaderVariableType::Vec2, ShaderVariableType::Vec2 });

			VBO->Unbind();
			VAO->Unbind();

			DefaultTexture = Renderer::CreateTexture().release();
			DefaultTexture->SetImage(Image::LoadFromFile("res/Circle.png"));
			DefaultTexture->Bind();

			CircleInstancedShader = Renderer::CreateShaderProgram("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag").release();

			InitilizedCircleVertices = true;
		}
	}

	void ParticleSystem::Update(const float& deltaTime)
	{
		SpawnAllParticlesOnQue(deltaTime);

		ActiveParticleCount = 0;
		for (Particle& particle : m_Particles) {

			if (Customizer.m_NoiseCustomizer.m_NoiseEnabled && particle.isActive) {
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		CircleInstancedShader->SetUniformMat4("projection", camera.ProjectionMatrix);
		CircleInstancedShader->SetUniformMat4("view", camera.ViewMatrix);
	}

	void ParticleSystem::Draw()
	{
		//bind vertex array and shader
		//glBindVertexArray(VAO);
		VAO->Bind();
		CircleInstancedShader->Bind();

		//set texture uniform (Sampler2D) to 0
		CircleInstancedShader->SetUniform1i("particleTexture", 0);

		//if we are using the default texture
		if (Customizer.m_TextureCustomizer.UseDefaultTexture)
			//bind the default texture to slot 0
			DefaultTexture->Bind(0);
		else
			//if we are using a custom texture, bind it to slot 0
			if(Customizer.m_TextureCustomizer.ParticleTexture)
				Customizer.m_TextureCustomizer.ParticleTexture->Bind(0);

		//cast the start of m_ParticleInfoBuffer to a glm::mat*, because the start of the buffer is the matrices
		glm::mat4* modelBuffer = (glm::mat4*) m_ParticleInfoBuffer;

		//cast the memory location of the part after the matrices to glm::vec4, because the colors(vec4s) are after the matrices in memory
		glm::vec4* colorBuffer = (glm::vec4*) ((char*)m_ParticleInfoBuffer + m_ParticleCount * sizeof(glm::mat4));

		//go through all the particles
		for (unsigned int i = 0; i < m_ParticleCount; i++)
		{
			//get a value from 0 to 1, showing how much the particle lived.
			//1 meaning it's lifetime is over and it is going to die (get deactivated and not rendered).
			//0 meaning it's just been spawned (activated).
			float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;

			//create a new model matrix for each particle
			glm::mat4 model = glm::mat4(1.0f);

			//if the particle is acive update it's model matrix
			if (m_Particles[i].isActive) {
				//move particle to it's position
				model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));
				//use the t value to get the scale of the particle using it's scale interpolator
				float scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);
				//scale the particle by that value
				model = glm::scale(model, glm::vec3(scale, scale, scale));

			}
			//if the particle is not active place it in a faraway place so it is not rendered
			else
			{
				model = glm::translate(model, glm::vec3(-10000, -10000, 0.0f));
			}
			//update the model
			modelBuffer[i] = model;
			//interpolate the color using the t value using the particles color interpolator
			colorBuffer[i] = m_Particles[i].m_ColorInterpolator.Interpolate(t);
		}

		//how many times we need to drae (currently we draw 40 particles at a time)
		int drawCount = (int)m_Particles.size() / 40;

		//draw 40 particles
		for (int i = 0; i < drawCount; i++)
		{
			CircleInstancedShader->SetUniformVec4s("colorArr", &colorBuffer[i * 40], 40);
			CircleInstancedShader->SetUniformMat4s("model", &modelBuffer[i * 40], 40);
			Renderer::DrawInstanced(*VAO, *CircleInstancedShader, Primitive::TriangleFan, 26, 40);
		}

		//get the remaining particles 
		int remaining = m_Particles.size() % 40;

		//draw them
		CircleInstancedShader->SetUniformVec4s("colorArr", &colorBuffer[drawCount * 40], remaining);
		CircleInstancedShader->SetUniformMat4s("model", &modelBuffer[drawCount * 40], remaining);
		Renderer::DrawInstanced(*VAO, *CircleInstancedShader, Primitive::TriangleFan, 26, remaining);
	}

	void ParticleSystem::SpawnParticle(const Particle& particle)
	{
		//go through all the particles
		for (unsigned int i = 0; i < m_ParticleCount; i++)
		{
			//find a particle that is not active
			if (!m_Particles[i].isActive)
			{
				//assign particle variables from the passed particle
				m_Particles[i].m_Position = particle.m_Position;
				m_Particles[i].m_ColorInterpolator = particle.m_ColorInterpolator;
				m_Particles[i].m_Velocity = particle.m_Velocity;
				m_Particles[i].isActive = true;
				m_Particles[i].m_ScaleInterpolator = particle.m_ScaleInterpolator;
				m_Particles[i].SetLifeTime(particle.m_LifeTime);

				//break out of the for loop because we are spawning one particle only
				break;
			}
		}

		//if no inactive particle is found, don't do anything (do not spawn a new particle)
	}

	void ParticleSystem::ClearParticles()
	{
		//deactivate all particles which will make them stop rendering
		for (Particle& m_particle : m_Particles)
			m_particle.isActive = false;
	}

	ParticleSystem::ParticleSystem(const ParticleSystem& Psystem) :
		Customizer(Psystem.Customizer)
	{
		//allocate space for a model matrix (glm::mat4) and a color (glm::vec4) for each particle system
		m_ParticleInfoBuffer = malloc((sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);
		//copy the info buffer from the object to be copied (Psystem) to the new object (this)
		memcpy(m_ParticleInfoBuffer, Psystem.m_ParticleInfoBuffer, (sizeof(glm::mat4) + sizeof(glm::vec4)) * Psystem.m_ParticleCount);

		//copy other variables
		m_Particles = Psystem.m_Particles;
		m_ParticleCount = Psystem.m_ParticleCount;
		m_Name = Psystem.m_Name;
		EditorOpen = Psystem.EditorOpen;
		ID = Psystem.ID;
		RenameTextOpen = Psystem.RenameTextOpen;

		//initilize the noise class
		m_Noise.Init();
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		//forward the call to the copy constructor
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGUI(Camera& camera)
	{
		if (EditorOpen)
			Customizer.DisplayGUI(m_Name, EditorOpen);

		//update editor line
		if (Customizer.Mode == SpawnMode::SpawnOnLine)
		{
			Customizer.m_Line.SetPoints(Customizer.m_LinePosition, Customizer.m_LineLength,Customizer.m_LineAngle);
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