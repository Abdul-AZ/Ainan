#include "FrameBuffer.h"

#include "Renderer.h"

namespace Ainan
{
    void FrameBufferNew::Resize(const glm::vec2& newSize)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::ResizeFrameBuffer;
        cmd.ResizeFrameBufferCmdDesc.Width = newSize.x;
        cmd.ResizeFrameBufferCmdDesc.Height = newSize.y;
        Renderer::Rdata->FrameBuffers[Identifier].Size = newSize;
        cmd.ResizeFrameBufferCmdDesc.Buffer = &Renderer::Rdata->FrameBuffers[Identifier];

        Renderer::PushCommand(cmd);
    }

    void* FrameBufferNew::GetTextureID()
    {
        //TODO clean this
        if(Renderer::Rdata->API == RendererType::D3D11)
            return (void*)Renderer::Rdata->FrameBuffers[Identifier].ResourceIdentifier;

        return (void*)Renderer::Rdata->FrameBuffers[Identifier].TextureIdentifier;
    }

    glm::vec2 FrameBufferNew::GetSize() const
    {
        return Renderer::Rdata->FrameBuffers[Identifier].Size;
    }

    Image* FrameBufferNew::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::ReadFrameBuffer;
        cmd.ReadFrameBufferCmdDesc.Buffer = &Renderer::Rdata->FrameBuffers[Identifier];
        cmd.ReadFrameBufferCmdDesc.Output = new Image;
        cmd.ReadFrameBufferCmdDesc.BottomLeftX = bottomLeftPixel.x;
        cmd.ReadFrameBufferCmdDesc.BottomLeftY = bottomLeftPixel.y;
        cmd.ReadFrameBufferCmdDesc.TopRightX = topRightPixel.x;
        cmd.ReadFrameBufferCmdDesc.TopRightY = topRightPixel.y;

        Renderer::PushCommand(cmd);
        Renderer::WaitUntilRendererIdle();
        return (Image*)cmd.ReadFrameBufferCmdDesc.Output;
    }

    void FrameBufferNew::Bind() const
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::BindFrameBufferAsRenderTarget;
        cmd.BindFrameBufferAsRenderTargetCmdDesc.Buffer = &Renderer::Rdata->FrameBuffers[Identifier];

        Renderer::PushCommand(cmd);
    }
}
