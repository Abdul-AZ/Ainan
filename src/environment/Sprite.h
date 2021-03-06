#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"
#include "file/AssetManager.h"
#include "renderer/Image.h"

namespace Ainan {

	class Sprite : public EnvironmentObjectInterface
	{
	public:
		Sprite();
		~Sprite();

		virtual void Update(const float deltaTime) override;
		virtual void Draw() override;
		virtual void DisplayGuiControls() override;
		int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION operation) override;

		void LoadTextureFromFile(const std::string& path);

	public:
		glm::vec4 Tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		std::filesystem::path m_TexturePath; //relative to the environment folder
	private:
		Texture m_Texture;
	};
}
