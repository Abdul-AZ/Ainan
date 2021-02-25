#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class LitSprite : public EnvironmentObjectInterface
	{
	public:
		LitSprite();
		~LitSprite();

		virtual void DisplayGuiControls() override;
		virtual void Draw() override;
		int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION operation) override;

	public: //TODO make it private
		struct LitSpriteUniformBuffer 
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			glm::vec4 Tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			float BaseLight = 0.1f;
			float MaterialConstantCoefficient = 1.0f;
			float MaterialLinearCoefficient = 0.15f;
			float MaterialQuadraticCoefficient = 0.02f;
		};

		VertexBuffer m_VertexBuffer;
		UniformBuffer m_UniformBuffer;
		glm::vec2 m_Position = glm::vec2(0.0f, 0.0f);
		LitSpriteUniformBuffer m_UniformBufferData;
		ObjSpace Space = OBJ_SPACE_2D;
	};

}