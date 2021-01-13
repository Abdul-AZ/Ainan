#include "VertexBuffer.h"

#include "RenderCommand.h"
#include "Renderer.h"

namespace Ainan
{
	void VertexBufferNew::UpdateData(int32_t offset, int32_t size, void* data)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateVertexBuffer;
		cmd.UpdateVertexBufferCmdDesc.Size = size;
		cmd.UpdateVertexBufferCmdDesc.Offset = offset;
		cmd.UpdateVertexBufferCmdDesc.Data = new uint8_t[size];
		memcpy(cmd.UpdateVertexBufferCmdDesc.Data, data, size);
		cmd.UpdateVertexBufferCmdDesc.VertexBuffer = &Renderer::Rdata->VertexBuffers[Identifier];

		Renderer::PushCommand(cmd);
	}

	uint32_t VertexBufferNew::GetUsedMemory() const
	{
		return Renderer::Rdata->VertexBuffers[Identifier].Size;
	}
}
