#pragma once

namespace Ainan {

	class ShaderProgram;
	class IndexBuffer;
	class Texture;
	struct Rectangle;

	enum class RendererType 
	{
#ifdef PLATFORM_WINDOWS
		D3D11,
#endif
		OpenGL
	};

	enum class Primitive
	{
		Triangles,
		TriangleFan,
		Lines
	};

	enum class RenderingBlendMode
	{
		Additive,
		Screen,
		NotSpecified //this will use the current mode it is set on
	};

	//pure virtual class (interface) for each renderer api to inherit from
	class RendererAPI
	{
	public:
		virtual void Draw(ShaderProgram& shader, const Primitive& mode,
						  const unsigned int& vertexCount) = 0;
		virtual void DrawInstanced(ShaderProgram& shader, const Primitive& mode,
								   const unsigned int& vertexCount, const unsigned int& objectCount) = 0;
		virtual void Draw(ShaderProgram& shader, const Primitive& mode,
						  const IndexBuffer& indexBuffer) = 0;
		virtual void Draw(ShaderProgram& shader, const Primitive& mode,
						  const IndexBuffer& indexBuffer, int vertexCount) = 0;

		virtual void ClearScreen() = 0;

		virtual void Present() = 0;

		virtual void SetBlendMode(RenderingBlendMode blendMode) = 0;

		virtual void SetViewport(const Rectangle& viewport) = 0;
		virtual Rectangle GetCurrentViewport() = 0;

		virtual void SetScissor(const Rectangle& scissor) = 0;
		virtual Rectangle GetCurrentScissor() = 0;

		virtual RendererType GetType() const = 0;
	};
}