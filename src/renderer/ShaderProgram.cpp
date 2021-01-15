#include "ShaderProgram.h"

#include "UniformBuffer.h"
#include "Framebuffer.h"
#include "Renderer.h"

namespace Ainan
{
	void ShaderProgram::BindUniformBuffer(UniformBuffer buffer, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindUniformBuffer;
		cmd.BindUniformBufferCmdDesc.Buffer = &Renderer::Rdata->UniformBuffers[buffer.Identifier];
		cmd.BindUniformBufferCmdDesc.Stage = stage;
		cmd.BindUniformBufferCmdDesc.Slot = slot;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgram::BindTexture(Texture texture, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindTexture;
		cmd.BindTextureProgramCmdDesc.Texture = &Renderer::Rdata->Textures[texture.Identifier];
		cmd.BindTextureProgramCmdDesc.Stage = stage;
		cmd.BindTextureProgramCmdDesc.Slot = slot;

		Renderer::PushCommand(cmd);
	}

	void ShaderProgram::BindTexture(Framebuffer fb, uint32_t slot, RenderingStage stage)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindFramebufferAsTexture;
		cmd.BindFramebufferAsTextureCmdDesc.Buffer = &Renderer::Rdata->Framebuffers[fb.Identifier];
		cmd.BindFramebufferAsTextureCmdDesc.Slot = slot;
		cmd.BindFramebufferAsTextureCmdDesc.Stage = stage;

		Renderer::PushCommand(cmd);
	}
}
