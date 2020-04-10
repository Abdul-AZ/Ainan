#include <pch.h>
#include "ParticleSystem.h"

namespace Ainan {
	static std::shared_ptr<Texture> DefaultTexture;

	ParticleSystem::ParticleSystem()
	{
		Type = EnvironmentObjectType::ParticleSystemType;

		m_Name = "Particle System";

		m_Noise.Init();

		//maximum of 2000 particles allowed for now
		m_ParticleCount = 2000;

		m_ParticleDrawTranslationBuffer.resize(m_ParticleCount);
		m_ParticleDrawScaleBuffer.resize(m_ParticleCount);
		m_ParticleDrawColorBuffer.resize(m_ParticleCount);
		m_Particles.resize(m_ParticleCount);

		//initilize the default shader
		if (DefaultTexture == nullptr) {
			DefaultTexture = Renderer::CreateTexture();
			DefaultTexture->SetImage(Image::LoadFromFile("res/Circle.png"));
			DefaultTexture->Bind();
		}
	}

	void ParticleSystem::Update(const float deltaTime)
	{
		SpawnAllParticlesOnQue(deltaTime);

		ActiveParticleCount = 0;
		for (Particle& particle : m_Particles) {

			//add noise if it is enabled
			if (Customizer.m_NoiseCustomizer.m_NoiseEnabled && particle.isActive) {
				particle.m_Velocity.x += m_Noise.Noise(particle.m_Position.x, particle.m_Position.y) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
				particle.m_Velocity.y += m_Noise.Noise(particle.m_Position.x + 30, particle.m_Position.y - 30) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
			}

			if (particle.isActive) {

				//add forces to the particle
				for (auto& force : Customizer.m_ForceCustomizer.m_Forces)
				{
					if (force.second.Enabled) 
						particle.m_Acceleration += force.second.GetEffect(particle.m_Position) * deltaTime;
				}

				//update particle speed, lifetime etc
				particle.Update(deltaTime);

				//limit particle velocity
				if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType != VelocityCustomizer::NoLimit)
				{
					//to make the code look cleaner
					VelocityCustomizer& velocityCustomizer = Customizer.m_VelocityCustomizer;

					//use normal velocity limit
					//by calculating the velocity in both x and y and limiting the length of the vector
					if (velocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::NormalLimit)
					{
						float length = glm::length(particle.m_Velocity);
						if (length > velocityCustomizer.m_MaxNormalVelocityLimit ||
							length < velocityCustomizer.m_MinNormalVelocityLimit)
						{
							glm::vec2 direction = glm::normalize(particle.m_Velocity);
							length = std::clamp(length, velocityCustomizer.m_MinNormalVelocityLimit, velocityCustomizer.m_MaxNormalVelocityLimit);
							particle.m_Velocity = length * direction;
						}
					}
					//limit velocity in each axis
					else if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::PerAxisLimit)
					{
						particle.m_Velocity.x = std::clamp(particle.m_Velocity.x, velocityCustomizer.m_MinPerAxisVelocityLimit.x, velocityCustomizer.m_MaxPerAxisVelocityLimit.x);
						particle.m_Velocity.y = std::clamp(particle.m_Velocity.y, velocityCustomizer.m_MinPerAxisVelocityLimit.y, velocityCustomizer.m_MaxPerAxisVelocityLimit.y);
					}
				}

				//update active particle count
				ActiveParticleCount++;
			}
		}
	}

	void ParticleSystem::Draw()
	{
		//reset the amount of particles to be drawn every frame
		m_ParticleDrawCount = 0;

		//go through all the particles
		for (unsigned int i = 0; i < m_ParticleCount; i++)
		{
			if (m_Particles[i].isActive) {

				//get a value from 0 to 1, showing how much the particle lived.
				//1 meaning it's lifetime is over and it is going to die (get deactivated and not rendered).
				//0 meaning it's just been spawned (activated).
				float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;

				//use the t value to get the scale of the particle using it's not using a Custom Curve
				float scale;
				if (m_Particles[i].m_ScaleInterpolator.Type == Custom)
					scale = m_Particles[i].CustomScaleCurve.Interpolate(m_Particles[i].m_ScaleInterpolator.startPoint, m_Particles[i].m_ScaleInterpolator.endPoint, t);
				else
					scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);

				//put the drawing properties of the particles in the draw buffers that would be drawn this frame
				m_ParticleDrawTranslationBuffer[m_ParticleDrawCount] = m_Particles[i].m_Position;
				m_ParticleDrawScaleBuffer[m_ParticleDrawCount] = scale;
				m_ParticleDrawColorBuffer[m_ParticleDrawCount] = m_Particles[i].m_ColorInterpolator.Interpolate(t);

				//up the amount of particles to be drawn this frame
				m_ParticleDrawCount++;
			}
		}

		if(Customizer.m_TextureCustomizer.UseDefaultTexture)
			Renderer::DrawQuadv(m_ParticleDrawTranslationBuffer.data(), m_ParticleDrawColorBuffer.data(),
				m_ParticleDrawScaleBuffer.data(), m_ParticleDrawCount, DefaultTexture);
		else
			Renderer::DrawQuadv(m_ParticleDrawTranslationBuffer.data(), m_ParticleDrawColorBuffer.data(),
				m_ParticleDrawScaleBuffer.data(), m_ParticleDrawCount, Customizer.m_TextureCustomizer.ParticleTexture);

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
				m_Particles[i].CustomScaleCurve = particle.CustomScaleCurve;
				m_Particles[i].SetLifeTime(particle.m_LifeTime);
				m_Particles[i].m_Acceleration = glm::vec2(0.0f);

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

	glm::vec2& ParticleSystem::GetPositionRef()
	{
		switch (Customizer.Mode)
		{
		case SpawnMode::SpawnOnPoint:
			return Customizer.m_SpawnPosition;

		case SpawnMode::SpawnOnLine:
			return Customizer.m_LinePosition;

		case SpawnMode::SpawnOnCircle:
		case SpawnMode::SpawnInsideCircle:
			return Customizer.m_CircleOutline.Position;

		}

		assert(false);
		return glm::vec2(0.0f);
	}

	ParticleSystem::ParticleSystem(const ParticleSystem& Psystem) :
		Customizer(Psystem.Customizer)
	{

		//these are calculated every frame, so there is no need to copy them. a resize should be enough
		m_ParticleDrawTranslationBuffer.resize(Psystem.m_ParticleDrawTranslationBuffer.size());
		m_ParticleDrawScaleBuffer.resize(Psystem.m_ParticleDrawScaleBuffer.size());
		m_ParticleDrawColorBuffer.resize(Psystem.m_ParticleDrawColorBuffer.size());

		//copy other variables
		m_Particles = Psystem.m_Particles;
		m_ParticleCount = Psystem.m_ParticleCount;
		m_Name = Psystem.m_Name;
		EditorOpen = Psystem.EditorOpen;
		RenameTextOpen = Psystem.RenameTextOpen;

		//initilize the noise class
		m_Noise.Init();
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		//forward the call to the copy constructor
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGUI()
	{
		ImGui::PushID(this);
		if (EditorOpen)
			Customizer.DisplayGUI(m_Name, EditorOpen);
		ImGui::PopID();

		//update editor line
		if (Customizer.Mode == SpawnMode::SpawnOnLine)
		{
			glm::vec2 pointOffset = Customizer.m_LineLength * glm::vec2(cos(glm::radians(Customizer.m_LineAngle)), sin(glm::radians(Customizer.m_LineAngle)));

			std::vector<glm::vec2> lineVertices;
			lineVertices.reserve(2);
			lineVertices.push_back(Customizer.m_LinePosition + pointOffset);
			lineVertices.push_back(Customizer.m_LinePosition - pointOffset);

			Customizer.m_Line.SetVertices(lineVertices);
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
}