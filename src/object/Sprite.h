#pragma once

#include "misc/EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"
#include "file/AssetManager.h"

namespace Ainan {

	class Sprite : public EnvironmentObjectInterface
	{
	public:
		Sprite();

		virtual void Update(const float deltaTime) override;
		virtual void Draw() override;
		virtual void DisplayGUI() override;
		virtual glm::vec2& GetPositionRef() override { return Position; };

		void LoadTextureFromFile(const std::string& path);

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		float Scale = 1.0f;
		float Rotation = 0.0f; //in degrees
		glm::vec4 Tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		std::string m_TexturePath;
	private:
		std::shared_ptr<Texture> m_Texture;
	};
}
