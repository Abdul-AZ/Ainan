#pragma once

#include "misc/Camera.h"

#include "RendererAPI.h"
#include "ShaderProgram.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Rectangle.h"

#include "opengl/OpenGLRendererAPI.h"
#include "opengl/OpenGLShaderProgram.h"
#include "opengl/OpenGLVertexArray.h"
#include "opengl/OpenGLVertexBuffer.h"
#include "opengl/OpenGLIndexBuffer.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/OpenGLFrameBuffer.h"

namespace Ainan {

	//this class is completely api agnostic, meaning NO gl calls, NO direct3D calls etc
	class Renderer 
	{
	public:
		//this will initilize all the shaders and put them in the shader library member
		static void Init();

		//this will destroy all the shaders in the shader library member
		static void Terminate();

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
		static void SetViewport(const Rectangle viewport);
		static Rectangle GetCurrentViewport();

		static std::shared_ptr<VertexArray> CreateVertexArray();

		static std::shared_ptr<VertexBuffer> CreateVertexBuffer(void* data, unsigned int size);

		//data should ALWAYS an UNSIGNED INT array
		static std::shared_ptr<IndexBuffer> CreateIndexBuffer(unsigned int* data, const int& count);

		//manually create a shader program (mostly used for testing new shaders)
		//to properly add shaders add them to the CompileOnInit list in the cpp file and access them from the ShaderLibrary member
		static std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);

		static std::shared_ptr<FrameBuffer> CreateFrameBuffer();

		static std::shared_ptr<Texture> CreateTexture();

		static Camera* m_CurrentSceneCamera;
		static unsigned int NumberOfDrawCallsLastScene;
		static glm::mat4 m_CurrentViewProjection;
		static RendererAPI* m_CurrentActiveAPI;
		static std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> ShaderLibrary;
	};

}
