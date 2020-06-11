#include <pch.h>
#include "ParticleSystem.h"

namespace Ainan {
	static std::shared_ptr<Texture> DefaultTexture;
	static int s_DefaultTextureUserCount = 0;

	ParticleSystem::ParticleSystem()
	{
		Type = EnvironmentObjectType::ParticleSystemType;

		m_Name = "Particle System";

		m_ParticleDrawTranslationBuffer.resize(c_ParticlePoolSize);
		m_ParticleDrawScaleBuffer.resize(c_ParticlePoolSize);
		m_ParticleDrawColorBuffer.resize(c_ParticlePoolSize);

		//initilize data for the particles
		m_Particles.IsActive.resize(c_ParticlePoolSize);
		m_Particles.Position.resize(c_ParticlePoolSize);
		m_Particles.Velocity.resize(c_ParticlePoolSize);
		m_Particles.Acceleration.resize(c_ParticlePoolSize);
		m_Particles.StartScale.resize(c_ParticlePoolSize);
		m_Particles.EndScale.resize(c_ParticlePoolSize);
		m_Particles.LifeTime.resize(c_ParticlePoolSize);
		m_Particles.RemainingLifeTime.resize(c_ParticlePoolSize);

		//initilize the default shader
		s_DefaultTextureUserCount++;
		if (s_DefaultTextureUserCount == 1)
		{
			DefaultTexture = Renderer::CreateTexture(Image::LoadFromFile("res/Circle.png"));
		}
	}

	ParticleSystem::~ParticleSystem()
	{
		s_DefaultTextureUserCount--;
		if (s_DefaultTextureUserCount == 0)
			DefaultTexture.reset();
	}

	void ParticleSystem::Update(const float deltaTime)
	{
		SpawnAllParticlesOnQue(deltaTime);

		ActiveParticleCount = 0;
		for (size_t i = 0; i < c_ParticlePoolSize; i++) {

			if(m_Particles.IsActive[i]) 
			{
				//do noise calculations
				Customizer.m_NoiseCustomizer.ApplyNoise(m_Particles.Position[i],
					m_Particles.Velocity[i],
					m_Particles.Acceleration[i],
					i);

				//add forces to the particle
				for (auto& force : Customizer.m_ForceCustomizer.m_Forces)
				{
					if (force.second.Enabled) 
						m_Particles.Acceleration[i] += force.second.GetEffect(m_Particles.Position[i]) * deltaTime;
				}

				//update particle speed, lifetime etc
				m_Particles.Velocity[i] += m_Particles.Acceleration[i];
				m_Particles.Position[i] += m_Particles.Velocity[i] * deltaTime;

				m_Particles.RemainingLifeTime[i] -= deltaTime;
				if (m_Particles.RemainingLifeTime[i] < 0.0f)
					m_Particles.IsActive[i] = false;


				//limit particle velocity
				if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType != VelocityCustomizer::NoLimit)
				{
					//to make the code look cleaner
					VelocityCustomizer& velocityCustomizer = Customizer.m_VelocityCustomizer;

					//use normal velocity limit
					//by calculating the velocity in both x and y and limiting the length of the vector
					if (velocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::NormalLimit)
					{
						float length = glm::length(m_Particles.Velocity[i]);
						if (length > velocityCustomizer.m_MaxNormalVelocityLimit ||
							length < velocityCustomizer.m_MinNormalVelocityLimit)
						{
							glm::vec2 direction = glm::normalize(m_Particles.Velocity[i]);
							length = std::clamp(length, velocityCustomizer.m_MinNormalVelocityLimit, velocityCustomizer.m_MaxNormalVelocityLimit);
							m_Particles.Velocity[i] = length * direction;
						}
					}
					//limit velocity in each axis
					else if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::PerAxisLimit)
					{
						m_Particles.Velocity[i].x = std::clamp(m_Particles.Velocity[i].x, velocityCustomizer.m_MinPerAxisVelocityLimit.x, velocityCustomizer.m_MaxPerAxisVelocityLimit.x);
						m_Particles.Velocity[i].y = std::clamp(m_Particles.Velocity[i].y, velocityCustomizer.m_MinPerAxisVelocityLimit.y, velocityCustomizer.m_MaxPerAxisVelocityLimit.y);
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
		for (size_t i = 0; i < c_ParticlePoolSize; i++)
		{
			if (m_Particles.IsActive[i]) 
			{

				//get a value from 0 to 1, showing how much the particle lived.
				//1 meaning it's lifetime is over and it is going to die (get deactivated and not rendered).
				//0 meaning it's just been spawned (activated).
				float t = (m_Particles.LifeTime[i] - m_Particles.RemainingLifeTime[i]) / m_Particles.LifeTime[i];

				//use the t value to get the scale of the particle using it's not using a Custom Curve
				float scale = 0.0f;
				if (Customizer.m_ScaleCustomizer.m_InterpolationType != Custom)
				{
					scale = 
						Interpolation::Interporpolate(Customizer.m_ScaleCustomizer.m_InterpolationType,
						m_Particles.StartScale[i],
						m_Particles.EndScale[i],
						t);
				}
				else
					scale = Customizer.m_ScaleCustomizer.m_Curve.Interpolate(m_Particles.StartScale[i], m_Particles.EndScale[i], t);

				//put the drawing properties of the particles in the draw buffers that would be drawn this frame
				m_ParticleDrawTranslationBuffer[m_ParticleDrawCount] = m_Particles.Position[i];
				m_ParticleDrawScaleBuffer[m_ParticleDrawCount] = scale;

				m_ParticleDrawColorBuffer[m_ParticleDrawCount] =
					Interpolation::Interporpolate(Customizer.m_ColorCustomizer.m_InterpolationType,
						Customizer.m_ColorCustomizer.StartColor,
						Customizer.m_ColorCustomizer.EndColor,
						t);

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

	void ParticleSystem::SpawnParticle(const ParticleDescription& particle)
	{
		//go through all the particles
		for (size_t i = 0; i < c_ParticlePoolSize; i++)
		{
			//find a particle that is not active
			if (!m_Particles.IsActive[i])
			{
				//assign particle variables from the passed particle
				m_Particles.Position[i] = particle.Position;
				m_Particles.Velocity[i] = particle.Velocity;
				m_Particles.IsActive[i] = true;
				m_Particles.StartScale[i] = particle.StartScale;
				m_Particles.EndScale[i] = particle.EndScale;
				m_Particles.LifeTime[i] = particle.LifeTime;
				m_Particles.RemainingLifeTime[i] = particle.LifeTime;
				m_Particles.Acceleration[i] = particle.Acceleration;

				//break out of the for loop because we are spawning one particle only
				break;
			}
		}

		//if no inactive particle is found, don't do anything (do not spawn a new particle)
	}

	void ParticleSystem::ClearParticles()
	{
		//deactivate all particles which will make them stop rendering
		m_Particles.IsActive.assign(m_Particles.IsActive.size(), false);
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
		m_Name = Psystem.m_Name;
		EditorOpen = Psystem.EditorOpen;
		RenameTextOpen = Psystem.RenameTextOpen;
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
		if (TimeTillNextParticleSpawn < 0.0f) 
		{
			TimeTillNextParticleSpawn = abs(TimeTillNextParticleSpawn);

			while (TimeTillNextParticleSpawn > 0.0f) 
			{
				ParticleDescription p = Customizer.GetParticleDescription();
				SpawnParticle(p);
				TimeTillNextParticleSpawn -= Customizer.GetTimeBetweenParticles();
			}

			TimeTillNextParticleSpawn = Customizer.GetTimeBetweenParticles();
		}
	}
}