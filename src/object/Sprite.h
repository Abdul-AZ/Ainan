#pragma once

#include "misc/EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"

namespace ALZ {

	class Sprite : public EnvironmentObjectInterface
	{
	public:
		Sprite();

		virtual void Update(const float& deltaTime) override;
		virtual void Draw() override;
		virtual void DisplayGUI() override;
		virtual glm::vec2& GetPositionRef() override { return Position; };

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);

	private:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<ShaderProgram> m_ShaderProgram;
		std::shared_ptr<Texture> m_Texture;
	};
}
