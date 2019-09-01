#pragma once

#include "misc/EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"
#include "file/FileBrowser.h"

namespace ALZ {

	class Sprite : public EnvironmentObjectInterface
	{
	public:
		Sprite();

		virtual void Update(const float& deltaTime) override;
		virtual void Draw() override;
		virtual void DisplayGUI() override;
		virtual glm::vec2& GetPositionRef() override { return Position; };

		void LoadTextureFromFile(const std::string& path);

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec2 Scale = glm::vec2(0.1f, 0.1f);
		float Rotation = 0.0f; //in degrees
		glm::vec4 Tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		std::string TextureImagePath = "";

	private:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<ShaderProgram> m_ShaderProgram;
		std::shared_ptr<Texture> m_Texture;
		FileBrowser m_FileBrowser;
	};
}
