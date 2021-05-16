#include "ParticleSystem.h"

namespace Ainan {
	static Texture DefaultTexture;
	static int s_DefaultTextureUserCount = 0;

	ParticleSystem::ParticleSystem()
	{
		Type = EnvironmentObjectType::ParticleSystemType;
		Space = OBJ_SPACE_2D;

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
		Customizer.m_SpawnPosition = ModelMatrix[3];
	}

	ParticleSystem::~ParticleSystem()
	{
		s_DefaultTextureUserCount--;
		if (s_DefaultTextureUserCount == 0)
			Renderer::DestroyTexture(DefaultTexture);
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

	void ParticleSystem::OnTransform()
	{
		Customizer.m_SpawnPosition = ModelMatrix[3];
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
		RenameTextOpen = Psystem.RenameTextOpen;
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		//forward the call to the copy constructor
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGuiControls()
	{
		DisplayTransformationControls();
		
		ImGui::NextColumn();
		IMGUI_DROPDOWN_START_USING_COLUMNS("Spawn Mode", GetModeAsText(Customizer.Mode).c_str());
		IMGUI_DROPDOWN_SELECTABLE(Customizer.Mode, SpawnMode::SpawnOnPoint, GetModeAsText(SpawnMode::SpawnOnPoint).c_str());
		IMGUI_DROPDOWN_SELECTABLE(Customizer.Mode, SpawnMode::SpawnOnLine, GetModeAsText(SpawnMode::SpawnOnLine).c_str());
		IMGUI_DROPDOWN_SELECTABLE(Customizer.Mode, SpawnMode::SpawnOnCircle, GetModeAsText(SpawnMode::SpawnOnCircle).c_str());
		IMGUI_DROPDOWN_SELECTABLE(Customizer.Mode, SpawnMode::SpawnInsideCircle, GetModeAsText(SpawnMode::SpawnInsideCircle).c_str());
		IMGUI_DROPDOWN_END();

		Customizer.m_TextureCustomizer.DisplayGUI();

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Emission"))
		{
			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Particles\nPer Second: ");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::DragFloat("##Particles\nPer Second: ", &Customizer.m_ParticlesPerSecond, 1.0f, 0.1f, 1000.0f);
			
			//limit to 1000
			Customizer.m_ParticlesPerSecond = std::clamp(Customizer.m_ParticlesPerSecond, 0.1f, 1000.0f);

			ImGui::NextColumn();
			ImGui::TreePop();
		}

		if (Customizer.Mode == SpawnMode::SpawnOnLine)
		{
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
			if (ImGui::TreeNode("Position"))
			{
				auto spacing = ImGui::GetCursorPosY();

				ImGui::Text("Line Length: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Line Length: ", &Customizer.m_LineLength, 0.001f);

				ImGui::NextColumn();
				ImGui::Text("Line Rotation :");
				ImGui::NextColumn();
				ImGui::DragFloat("##Line Rotation: ", &Customizer.m_LineAngle, 1.0f, 0.0f, 360.0f);

				ImGui::NextColumn();
				ImGui::TreePop();
			}
		}
		else if (Customizer.Mode == SpawnMode::SpawnOnCircle || Customizer.Mode == SpawnMode::SpawnInsideCircle)
		{
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
			if (ImGui::TreeNode("Position"))
			{
				auto spacing = ImGui::GetCursorPosY();

				ImGui::Text("Circle Radius: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Circle Radius: ", &Customizer.m_CircleRadius, 0.001f);

				Customizer.m_CircleRadius = std::clamp(Customizer.m_CircleRadius, 0.001f, 10000.0f);

				ImGui::NextColumn();
				ImGui::TreePop();
			}
		}

		Customizer.m_NoiseCustomizer.DisplayGUI();
		Customizer.m_VelocityCustomizer.DisplayGUI();
		Customizer.m_ColorCustomizer.DisplayGUI();
		Customizer.m_LifetimeCustomizer.DisplayGUI();
		Customizer.m_ScaleCustomizer.DisplayGUI();
		Customizer.m_ForceCustomizer.DisplayGUI();
	}

	int32_t ParticleSystem::GetAllowedGizmoOperation(ImGuizmo::OPERATION operation)
	{
		if (Space == OBJ_SPACE_2D)
		{
			if (operation == ImGuizmo::OPERATION::TRANSLATE)
				return ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
			else if (operation == ImGuizmo::OPERATION::ROTATE || operation == ImGuizmo::OPERATION::SCALE)
				return ImGuizmo::OPERATION::BOUNDS; //work around for not allowing any operation
		}
		else if (Space == OBJ_SPACE_3D)
		{
			return operation;
		}

		AINAN_LOG_ERROR("Invalid Gizmo Operation Given");
		return -1;
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