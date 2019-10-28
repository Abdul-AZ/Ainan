#include <pch.h>

#include "Sprite.h"

namespace Ainan {

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
		glm::mat4 u_Model(1.0f);
		u_Model = glm::translate(u_Model, glm::vec3(Position.x, Position.y, 0.0f) * GlobalScaleFactor);
		u_Model = glm::rotate(u_Model, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		u_Model = glm::scale(u_Model, glm::vec3(Scale.x, Scale.y, 1.0f) * GlobalScaleFactor);

		auto& shader = Renderer::ShaderLibrary["SpriteShader"];

		shader->SetUniformMat4("u_Model", u_Model);
		shader->SetUniform1i("u_SpriteTexture", 0);
		shader->SetUniformVec4("u_Tint", Tint);
		m_Texture->Bind(0);

		Renderer::Draw(*m_VertexArray, *shader, Primitive::Triangles, 6);

		m_Texture->Unbind();
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

		if (ImGui::BeginCombo("##Texture: ", m_TexturePath == "" ? "None" : std::filesystem::path(m_TexturePath).filename().u8string().c_str()))
		{
			auto textures = AssetManager::GetAll2DTextures();
			bool selected = false;
			if (ImGui::Selectable("None", &selected))
			{
				LoadTextureFromFile("res/CheckerBoard.png");
				m_TexturePath = "";
			}
			for (auto& tex : textures)
			{
				std::string textureFileName = std::filesystem::path(tex).filename().u8string();
				if (ImGui::Selectable(textureFileName.c_str(), &selected))
				{
					if (textureFileName != "Default")
					{
						LoadTextureFromFile(tex);
						std::string absolutePathToEnv = AssetManager::GetAbsolutePath();
						m_TexturePath = tex.substr(absolutePathToEnv.size(), tex.size() - absolutePathToEnv.size());
					}
				}
			}

			ImGui::EndCombo();
		}



		ImGui::Spacing();

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::DragFloat2("##Position: ", &Position.x, 0.01f);

		ImGui::Text("Scale: ");
		ImGui::SameLine();
		ImGui::DragFloat2("##Scale: ", &Scale.x, 0.01f);

		ImGui::Text("Rotate: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Rotate: ", &Rotation);
		Rotation = std::clamp(Rotation, 0.0f, 360.0f);

		ImGui::Text("Tint: ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##Tint: ", &Tint.r);
		

		ImGui::End();

		ImGui::PopID();
	}

	void Sprite::LoadTextureFromFile(const std::string& path)
	{
		m_Texture.reset();

		m_Texture = Renderer::CreateTexture();

		Image img = Image::LoadFromFile(path);

		if (img.m_Comp == 1)
			Image::GrayScaleToRGB(img);

		m_Texture->SetImage(img);
	}

}