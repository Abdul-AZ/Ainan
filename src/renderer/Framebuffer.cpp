#include "Framebuffer.h"

#include "Renderer.h"

namespace Ainan
{
    void Framebuffer::Resize(const glm::vec2& newSize)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::ResizeFramebuffer;
        cmd.ResizeFramebufferCmdDesc.Width = newSize.x;
        cmd.ResizeFramebufferCmdDesc.Height = newSize.y;
        Renderer::Rdata->Framebuffers[Identifier].Size = newSize;
        cmd.ResizeFramebufferCmdDesc.Buffer = &Renderer::Rdata->Framebuffers[Identifier];

        Renderer::PushCommand(cmd);
    }

    void* Framebuffer::GetTextureID()
    {
#ifdef PLATFORM_WINDOWS
        //TODO clean this
        if(Renderer::Rdata->API == RendererType::D3D11)
            return (void*)Renderer::Rdata->Framebuffers[Identifier].ResourceIdentifier;
#endif
        return (void*)Renderer::Rdata->Framebuffers[Identifier].TextureIdentifier;
    }

    glm::vec2 Framebuffer::GetSize() const
    {
        return Renderer::Rdata->Framebuffers[Identifier].Size;
    }

    Image* Framebuffer::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::ReadFramebuffer;
        cmd.ReadFramebufferCmdDesc.Buffer = &Renderer::Rdata->Framebuffers[Identifier];
        cmd.ReadFramebufferCmdDesc.Output = new Image;
        cmd.ReadFramebufferCmdDesc.BottomLeftX = bottomLeftPixel.x;
        cmd.ReadFramebufferCmdDesc.BottomLeftY = bottomLeftPixel.y;
        cmd.ReadFramebufferCmdDesc.TopRightX = topRightPixel.x;
        cmd.ReadFramebufferCmdDesc.TopRightY = topRightPixel.y;

        Renderer::PushCommand(cmd);
        Renderer::WaitUntilRendererIdle();
        return (Image*)cmd.ReadFramebufferCmdDesc.Output;
    }

    void Framebuffer::Bind() const
    {
        RenderCommand cmd;
        cmd.Type = RenderCommandType::BindFramebufferAsRenderTarget;
        cmd.BindFramebufferAsRenderTargetCmdDesc.Buffer = &Renderer::Rdata->Framebuffers[Identifier];

        Renderer::PushCommand(cmd);
    }
}
