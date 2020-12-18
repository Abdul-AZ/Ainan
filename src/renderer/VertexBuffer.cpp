#include "VertexBuffer.h"

#include "RenderCommand.h"
#include "Renderer.h"

namespace Ainan
{
	void VertexBufferNew::UpdateData(int32_t offset, int32_t size, void* data)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateVertexBuffer;
		cmd.Misc1 = size;
		cmd.Misc2 = offset;
		cmd.ExtraData = new uint8_t[size];
		memcpy(cmd.ExtraData, data, size);
		cmd.NewVBuffer = Renderer::Rdata->VertexBuffers[Identifier];

		Renderer::PushCommand(cmd);
	}
}
