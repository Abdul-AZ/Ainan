#pragma once

#include "Camera.h"

#include "graphics/RendererAPI.h"
#include "graphics/ShaderProgram.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Texture.h"
#include "graphics/FrameBuffer.h"

#include "opengl/OpenGLRendererAPI.h"
#include "opengl/OpenGLShaderProgram.h"
#include "opengl/OpenGLVertexArray.h"
#include "opengl/OpenGLVertexBuffer.h"
#include "opengl/OpenGLIndexBuffer.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/OpenGLFrameBuffer.h"

namespace ALZ {

	//this class is completely api agnostic, meaning NO gl calls, NO direct3D calls etc
	class Renderer 
	{
	public:
		static void Init();

		//This will be changed to only render in end scene by putting draw commands to a command buffer 
		static void BeginScene(Camera& camera);
		static void EndScene();

		//these will be changed to SubmitRenderCommand() or something like that

		//these overloads DO NOT use an index buffer
		static void Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode,
						 const unsigned int& vertexCount);
		static void DrawInstanced(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode,
								  const unsigned int& vertexCount, const unsigned int& objectCount);

		//these overloads DO use an index buffer
		static void Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& primitive,
						 const IndexBuffer& indexBuffer);

		static void ClearScreen();

		static std::unique_ptr<VertexArray> CreateVertexArray();
		static std::unique_ptr<VertexBuffer> CreateVertexBuffer(void* data, unsigned int size);
		//data should ALWAYS an UNSIGNED INT array
		static std::unique_ptr<IndexBuffer> CreateIndexBuffer(unsigned int* data, const int& count);
		static std::unique_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);
		static std::unique_ptr<FrameBuffer> CreateFrameBuffer();
		static std::unique_ptr<Texture> CreateTexture();
	private:
		static Camera* m_Camera;
		static RendererAPI* m_CurrentActiveAPI;
	};

}
