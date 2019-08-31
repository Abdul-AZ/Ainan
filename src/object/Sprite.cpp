#include <pch.h>

#include "Sprite.h"

namespace ALZ {

	Sprite::Sprite()
	{
		Type = SpriteType;
		m_Name = "Sprite";
		EditorOpen = false;

		m_VertexArray = Renderer::CreateVertexArray();
		m_VertexArray->Bind();

		glm::vec2 vertices[] = {
								//Positions				//Texture Coordinates
								glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0),
								glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 1.0),
								glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),

								glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),
								glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 0.0),
								glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0)
		};
		
		m_VertexBuffer = Renderer::CreateVertexBuffer(vertices, sizeof(vertices));
		m_VertexBuffer->Bind();
		m_VertexBuffer->SetLayout({ ShaderVariableType::Vec2, ShaderVariableType::Vec2 });

		m_ShaderProgram = Renderer::CreateShaderProgram("shaders/Sprite.vert", "shaders/Sprite.frag");

		m_Texture = Renderer::CreateTexture();

		Image img = Image::LoadFromFile("res/CheckerBoard.png");

		Image::GrayScaleToRGB(img);

		m_Texture->SetImage(img);

	}

	void Sprite::Update(const float& deltaTime)
	{
	}

	void Sprite::Draw()
	{
	}

	void Sprite::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen, ImGuiWindowFlags_NoSavedSettings);

		ImGui::Text("Texture: ");
		ImGui::SameLine();
		ImGui::Image((ImTextureID)m_Texture->GetRendererID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

		ImGui::Spacing();

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::DragFloat2("##Position: ", &Position.x);

		ImGui::End();

		ImGui::PopID();
	}

}