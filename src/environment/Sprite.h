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

		virtual void Update(const float deltaTime) override;
		virtual void Draw() override;
		virtual void DisplayGUI() override;
		virtual glm::vec2* GetPositionRef() override { return &Position; };

		void LoadTextureFromFile(const std::string& path);

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		float Scale = 1.0f;
		float Rotation = 0.0f; //in degrees
		glm::vec4 Tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		std::filesystem::path m_TexturePath; //relative to the environment folder
	private:
		TextureNew m_Texture;
	};
}
