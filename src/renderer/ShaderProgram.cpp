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
		cmd.ExtraData = &Renderer::Rdata->UniformBuffers[buffer.Identifier];
		cmd.Misc1 = slot;
		cmd.Misc2 = (uint32_t)stage;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindTexture;
		cmd.NewShader = Renderer::Rdata->ShaderPrograms[Identifier];
		cmd.Tex = texture;
		cmd.Misc1 = slot;
		cmd.Stage = stage;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(FrameBufferNew fb, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindFrameBufferAsTexture;
		cmd.NewShader = Renderer::Rdata->ShaderPrograms[Identifier];
		cmd.NewFBuffer = Renderer::Rdata->FrameBuffers[fb.Identifier];
		cmd.Misc1 = slot;
		cmd.Stage = stage;

		Renderer::PushCommand(cmd);
	}
}
