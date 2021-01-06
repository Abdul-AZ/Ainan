#include "UniformBuffer.h"

#include "RenderCommand.h"
#include "Renderer.h"

namespace Ainan
{
	void UniformBufferNew::UpdateData(void* data, uint32_t packedDataSizeofBuffer)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateUniformBuffer;
		cmd.UniformBuffer = &Renderer::Rdata->UniformBuffers[Identifier];
		void* dataCpy = new uint8_t[packedDataSizeofBuffer];
		memcpy(dataCpy, data, packedDataSizeofBuffer);
		cmd.ExtraData = dataCpy;
		Renderer::PushCommand(cmd);
	}
}
