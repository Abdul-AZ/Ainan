#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class LitSprite : public EnvironmentObjectInterface
	{
	public:
		LitSprite();

		virtual glm::vec2* GetPositionRef() override { return &m_Position; };

	private:
		struct LitSpriteUniformBuffer 
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			glm::vec4 BaseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			float BaseLight = 0.1f;
			float MaterialConstantCoefficient = 1.0f;
			float MaterialLinearCoefficient = 0.15f;
			float MaterialQuadraticCoefficient = 0.02f;
		};

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<UniformBuffer> m_UniformBuffer;
		glm::vec2 m_Position = glm::vec2(0.0f, 0.0f);
	};

}