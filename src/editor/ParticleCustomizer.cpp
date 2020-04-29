#include <pch.h>
#include "ParticleCustomizer.h"

namespace Ainan {

	ParticleCustomizer::ParticleCustomizer() :
		mt(std::random_device{}())
	{
		m_Line.Color = glm::vec4(0.0f, 0.7f, 0.0f, 0.85f);
		m_CircleOutline.Color = glm::vec4(0.0f, 0.7f, 0.0f, 0.85f);
		m_CircleOutline.Position = { 0.5f, 0.5f };
	}

	ParticleCustomizer::ParticleCustomizer(const ParticleCustomizer& customizer) :
		m_TextureCustomizer(customizer.m_TextureCustomizer)
	{
		m_VelocityCustomizer = customizer.m_VelocityCustomizer;
		m_NoiseCustomizer = customizer.m_NoiseCustomizer;
		m_LifetimeCustomizer = customizer.m_LifetimeCustomizer;
		m_ColorCustomizer = customizer.m_ColorCustomizer;
		m_ScaleCustomizer = customizer.m_ScaleCustomizer;
		m_ForceCustomizer = customizer.m_ForceCustomizer;
		m_Line = customizer.m_Line;
		m_LineAngle = customizer.m_LineAngle;
		m_LinePosition = customizer.m_LinePosition;
		m_CircleOutline = customizer.m_CircleOutline;
		mt = customizer.mt;
	}

	ParticleCustomizer ParticleCustomizer::operator=(const ParticleCustomizer& customizer)
	{
		return ParticleCustomizer(customizer);
	}

	std::string GetModeAsText(const SpawnMode& mode)
	{
		switch (mode)
		{
		case SpawnMode::SpawnOnPoint:
			return "Spawn On Point";

		case SpawnMode::SpawnOnLine:
			return "Spawn On Line";

		case SpawnMode::SpawnOnCircle:
			return "Spawn On Circle";

		case SpawnMode::SpawnInsideCircle:
			return "Spawn Inside Circle";

		default:
			return "";
		}
	}

	SpawnMode GetTextAsMode(const std::string& mode)
	{
		if (mode == "Spawn On Point")
			return SpawnMode::SpawnOnPoint;
		else if (mode == "Spawn On Line")
			return SpawnMode::SpawnOnLine;
		else if (mode == "Spawn On Circle")
			return SpawnMode::SpawnOnCircle;
		else if (mode == "Spawn Inside Circle")
			return SpawnMode::SpawnInsideCircle;
		else {
			assert(false);
			return SpawnMode::SpawnOnPoint;
		}
	}

	void ParticleCustomizer::DisplayGUI(const std::string& windowName, bool& windowOpen)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(575.0f, 500.0f), ImVec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().max()));
		ImGui::Begin((windowName.size() > 0) ? (windowName + "##" +  std::to_string(ImGui::GetID(this))).c_str() : "No Name", &windowOpen, ImGuiWindowFlags_NoSavedSettings);

		ImGui::Text("Spawn\n Mode");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Spawn Mode", GetModeAsText(Mode).c_str())) {

			{
				bool is_active = Mode == SpawnMode::SpawnOnPoint;
				if (ImGui::Selectable(GetModeAsText(SpawnMode::SpawnOnPoint).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnPoint;
				}
			}

			{
				bool is_active = Mode == SpawnMode::SpawnOnLine;
				if (ImGui::Selectable(GetModeAsText(SpawnMode::SpawnOnLine).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnLine;
				}
			}

			{
				bool is_active = Mode == SpawnMode::SpawnOnCircle;
				if (ImGui::Selectable(GetModeAsText(SpawnMode::SpawnOnCircle).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnOnCircle;
				}
			}

			{
				bool is_active = Mode == SpawnMode::SpawnInsideCircle;
				if (ImGui::Selectable(GetModeAsText(SpawnMode::SpawnInsideCircle).c_str(), &is_active)) {
					ImGui::SetItemDefaultFocus();
					Mode = SpawnMode::SpawnInsideCircle;
				}
			}

			ImGui::EndCombo();
		}

		m_TextureCustomizer.DisplayGUI();

		if (ImGui::TreeNode("Emission")) {

			ImGui::Text("Particles\nPer Second: ");
			ImGui::SameLine();
			ImGui::DragFloat("##Particles\nPer Second: ", &m_ParticlesPerSecond, 1.0f, 0.1f, 1000.0f);

			ImGui::TreePop();
		}

		if (Mode == SpawnMode::SpawnOnPoint) {
			if (ImGui::TreeNode("Position")) {

				ImGui::Text("Starting Position:");
				ImGui::SameLine();
				ImGui::DragFloat2("##Starting Position:", &m_SpawnPosition.x, 0.001f);

				ImGui::TreePop();
			}
		}
		else if (Mode == SpawnMode::SpawnOnLine)
		{
			if (ImGui::TreeNode("Position")) {

				ImGui::Text("Line Position: ");
				ImGui::SameLine();
				float xPos = ImGui::GetCursorPosX();
				ImGui::DragFloat2("##Line Position: ", &m_LinePosition.x, 0.001f);

				ImGui::Text("Line Length: ");
				ImGui::SameLine();
				ImGui::SetCursorPosX(xPos);
				ImGui::DragFloat("##Line Length: ", &m_LineLength, 0.001f);

				ImGui::Text("Line Rotation :");
				ImGui::SameLine();
				ImGui::SetCursorPosX(xPos);
				ImGui::DragFloat("##Line Rotation: ", &m_LineAngle, 1.0f, 0.0f, 360.0f);

				ImGui::TreePop();
			}
		}
		else if (Mode == SpawnMode::SpawnOnCircle || Mode == SpawnMode::SpawnInsideCircle)
		{
			if (ImGui::TreeNode("Position")) 
			{

				ImGui::Text("Circle Position: ");
				ImGui::SameLine();
				float xPos = ImGui::GetCursorPosX();
				ImGui::DragFloat2("##Circle Position: ", &m_CircleOutline.Position.x, 0.001f);

				ImGui::Text("Circle Radius: ");
				ImGui::SameLine();
				ImGui::SetCursorPosX(xPos);
				ImGui::DragFloat("##Circle Radius: ", &m_CircleOutline.Radius, 0.001f);

				m_CircleOutline.Radius = std::clamp(m_CircleOutline.Radius, 0.001f, 10000.0f);

				ImGui::TreePop();
			}
		}

		m_NoiseCustomizer.DisplayGUI();
		m_VelocityCustomizer.DisplayGUI();
		m_ColorCustomizer.DisplayGUI();
		m_LifetimeCustomizer.DisplayGUI();
		m_ScaleCustomizer.DisplayGUI();
		m_ForceCustomizer.DisplayGUI();

		ImGui::End();
	}

	ParticleDescription ParticleCustomizer::GetParticleDescription()
	{
		ParticleDescription particleDesc = {};

		switch (Mode)
		{
		case SpawnMode::SpawnOnPoint: {
			glm::vec2 spawnPosition = { m_SpawnPosition.x * c_GlobalScaleFactor, m_SpawnPosition.y * c_GlobalScaleFactor };
			particleDesc.Position = spawnPosition;
			break;
		}

		case SpawnMode::SpawnOnLine: {
			std::uniform_real_distribution<float> dest(0.0f, 1.0f);
			particleDesc.Position = m_Line.GetPointInLine(dest(mt));
			break;
		}

		case SpawnMode::SpawnOnCircle: {
			//random angle between 0 and 2pi (360 degrees)
			std::uniform_real_distribution<float> dest(0.0f, 2.0f * 3.14159f);
			particleDesc.Position = m_CircleOutline.GetPointByAngle(dest(mt));
			break;
		}

		case SpawnMode::SpawnInsideCircle: {
			std::uniform_real_distribution<float> dest(0.0f, 1.0f);
			float r = m_CircleOutline.Radius * sqrt(dest(mt));
			float theta = dest(mt) * 2 * PI; //in radians
			particleDesc.Position = glm::vec2(m_CircleOutline.Position.x + r * cos(theta), m_CircleOutline.Position.y + r * sin(theta));
			particleDesc.Position *= c_GlobalScaleFactor;
			break;
		}
		}

		particleDesc.Velocity = m_VelocityCustomizer.GetVelocity();
		particleDesc.LifeTime = m_LifetimeCustomizer.GetLifetime();

		particleDesc.StartScale = m_ScaleCustomizer.GetScaleInterpolator().startPoint;
		particleDesc.EndScale = m_ScaleCustomizer.GetScaleInterpolator().endPoint;

		return particleDesc;
	}
}