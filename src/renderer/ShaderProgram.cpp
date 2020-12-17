#include "ShaderProgram.h"

#include "UniformBuffer.h"
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
}
