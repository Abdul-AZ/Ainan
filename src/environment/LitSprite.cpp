#include <pch.h>

#include "LitSprite.h"

namespace Ainan {

	LitSprite::LitSprite()
	{
		Type = LitSpriteType;
		auto vertices = Renderer::GetQuadVertices();

		VertexLayout vertexBufferlayout = 
		{
			{ "aPos", ShaderVariableType::Vec2 } 
		};
		m_VertexBuffer = Renderer::CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(glm::vec2), vertexBufferlayout, Renderer::ShaderLibrary()["BackgroundShader"]);

		VertexLayout uniformBufferLayout =
		{
			{ "u_Model"    , ShaderVariableType::Mat4  },
			{ "u_BaseColor", ShaderVariableType::Vec3  },
			{ "u_BaseLight", ShaderVariableType::Float },
			{ "u_Constant" , ShaderVariableType::Float },
			{ "u_Linear"   , ShaderVariableType::Float },
			{ "u_Quadratic", ShaderVariableType::Float }
		};

		m_UniformBuffer = Renderer::CreateUniformBuffer("LightingData", 2, uniformBufferLayout, nullptr);
	}
}