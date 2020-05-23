#pragma once

namespace Ainan {

	//ONLY create this using Renderer::CreateShaderProgram
	class ShaderProgram
	{
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual void BindUniformBuffer(const char* name, uint32_t slot) = 0;
		virtual void SetUniform1i(const char* name, const int& value) = 0;
		
		virtual int GetUniformLocation(const char* name) = 0;
		virtual int GetRendererID() const = 0;
	};
}