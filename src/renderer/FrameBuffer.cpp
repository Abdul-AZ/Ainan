#include "FrameBuffer.h"

#include "Renderer.h"

namespace Ainan
{
    void FrameBufferNew::Blit(FrameBufferNew* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::BlitFrameBuffer;
        cmd.NewFBuffer = Renderer::Rdata->FrameBuffers[Identifier];
        if (otherBuffer)
        {
            cmd.ExtraData = new FrameBufferDataView;
            memcpy(cmd.ExtraData, &Renderer::Rdata->FrameBuffers[otherBuffer->Identifier], sizeof(FrameBufferDataView));
        }
        cmd.ExtraData = nullptr;

        memcpy(&cmd.Misc1, &sourceSize, sizeof(glm::vec2));
        memcpy(&cmd.Misc2, &targetSize, sizeof(glm::vec2));

        Renderer::PushCommand(cmd);
    }

    void FrameBufferNew::Resize(const glm::vec2& newSize)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::ResizeFrameBuffer;
        cmd.Misc1 = newSize.x;
        cmd.Misc2 = newSize.y;
        Renderer::Rdata->FrameBuffers[Identifier].Size = newSize;
        cmd.NewFBuffer = Renderer::Rdata->FrameBuffers[Identifier];

        Renderer::PushCommand(cmd);
    }

    void* FrameBufferNew::GetTextureID()
    {
        return (void*)Renderer::Rdata->FrameBuffers[Identifier].TextureIdentifier;
    }

    glm::vec2 FrameBufferNew::GetSize() const
    {
        return Renderer::Rdata->FrameBuffers[Identifier].Size;
    }

    void FrameBufferNew::Bind() const
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::BindFrameBufferAsRenderTarget;
        cmd.NewFBuffer = Renderer::Rdata->FrameBuffers[Identifier];

        Renderer::PushCommand(cmd);
    }
}
