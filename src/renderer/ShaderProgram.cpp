#include "ShaderProgram.h"

#include "UniformBuffer.h"
#include "FrameBuffer.h"
#include "Renderer.h"

namespace Ainan
{
	void ShaderProgramNew::BindUniformBuffer(UniformBufferNew buffer, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindUniformBuffer;
		cmd.UniformBuffer = &Renderer::Rdata->UniformBuffers[buffer.Identifier];
		cmd.Stage = stage;
		cmd.Misc1 = slot;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(TextureNew texture, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindTexture;
		cmd.Shader = &Renderer::Rdata->ShaderPrograms[Identifier];
		cmd.NewTex = &Renderer::Rdata->Textures[texture.Identifier];
		cmd.Misc1 = slot;
		cmd.Stage = stage;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(FrameBufferNew fb, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindFrameBufferAsTexture;
		cmd.Shader = &Renderer::Rdata->ShaderPrograms[Identifier];
		cmd.FrameBuffer = &Renderer::Rdata->FrameBuffers[fb.Identifier];
		cmd.Misc1 = slot;
		cmd.Stage = stage;

		Renderer::PushCommand(cmd);
	}
}
