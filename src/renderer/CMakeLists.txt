target_sources(Core PUBLIC

    "Renderer.h"            "Renderer.cpp"
    "RendererAPI.h"
    "RendererContext.h"
    "RenderCommand.h"       "RenderCommand.cpp"
    "RenderCommandQueue.h"  "RenderCommandQueue.cpp"
    "VertexBuffer.h"        "VertexBuffer.cpp"
    "IndexBuffer.h"
    "UniformBuffer.h"       "UniformBuffer.cpp"
    "ShaderProgram.h"       "ShaderProgram.cpp"
    "Texture.h"             "Texture.cpp"
    "Rectangle.h"
    "Framebuffer.h"         "Framebuffer.cpp"
    "Image.h"               "Image.cpp"
    "Camera.h"              "Camera.cpp"
    "RenderSurface.h"       "RenderSurface.cpp"
    
    "opengl/OpenGLRendererAPI.h"      "opengl/OpenGLRendererAPI.cpp"
    "opengl/OpenGLRendererContext.h"  "opengl/OpenGLRendererContext.cpp"
)