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
		cmd.BindUniformBufferCmdDesc.Buffer = &Renderer::Rdata->UniformBuffers[buffer.Identifier];
		cmd.BindUniformBufferCmdDesc.Stage = stage;
		cmd.BindUniformBufferCmdDesc.Slot = slot;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(TextureNew texture, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindTexture;
		cmd.BindTextureProgramCmdDesc.Texture = &Renderer::Rdata->Textures[texture.Identifier];
		cmd.BindTextureProgramCmdDesc.Stage = stage;
		cmd.BindTextureProgramCmdDesc.Slot = slot;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgramNew::BindTexture(FrameBufferNew fb, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindFrameBufferAsTexture;
		cmd.BindFrameBufferAsTextureCmdDesc.Buffer = &Renderer::Rdata->FrameBuffers[fb.Identifier];
		cmd.BindFrameBufferAsTextureCmdDesc.Slot = slot;
		cmd.BindFrameBufferAsTextureCmdDesc.Stage = stage;

		Renderer::PushCommand(cmd);
	}
}
