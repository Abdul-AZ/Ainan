#pragma once

#include "Texture.h"

namespace Ainan {

	enum class RenderingStage : uint32_t
	{
		VertexShader,
		FragmentShader
	};

	class Texture;
	class UniformBuffer;
	class Framebuffer;
	class UniformBuffer;
	class Framebuffer;

	class ShaderProgram
	{
	public:
		//This is used by the renderer to interact with the abstracted uniform buffer
		uint32_t Identifier = 0;

		void BindUniformBuffer(UniformBuffer buffer, uint32_t slot, RenderingStage stage);
		void BindTexture(Texture texture, uint32_t slot, RenderingStage stage);
		void BindTexture(Framebuffer fb, uint32_t slot, RenderingStage stage);
	};

	struct ShaderProgramDataView
	{
		uint64_t Identifier = 0;
		uint64_t Identifier_1 = 0;
		bool Deleted = false;
		uint8_t* VertexByteCode = nullptr;
		uint32_t VertexByteCodeSize = 0;
	};
}