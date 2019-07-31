#pragma once

namespace ALZ {

	class ShaderProgram;
	class IndexBuffer;
	class Texture;
	struct Viewport;

	enum class RendererType {
		OpenGL
	};

	enum class Primitive
	{
		Triangles,
		TriangleFan,
		Lines
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
		virtual void ClearScreen() = 0;
		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual Viewport GetCurrentViewport() = 0;
		virtual RendererType GetType() const = 0;
	};


}