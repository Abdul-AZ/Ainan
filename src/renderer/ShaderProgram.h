#pragma once

namespace Ainan {

	//ONLY create this using Renderer::CreateShaderProgram
	class ShaderProgram
	{
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual void BindUniformBuffer(const char* name, uint32_t slot) = 0;
		virtual void SetUniform1i(const char* name, const int& value)                                  = 0;
		virtual void SetUniform1f(const char* name, const float& value)                                = 0;
		virtual void SetUniform1fs(const char* name, float * value, const int& count)                  = 0;
		virtual void SetUniformVec2(const char* name, const glm::vec2& value)                          = 0;
		virtual void SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count) = 0;
		virtual void SetUniformVec3(const char* name, const glm::vec3& value)                          = 0;
		virtual void SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count) = 0;
		virtual void SetUniformVec4(const char* name, const glm::vec4& value)                          = 0;
		virtual void SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count) = 0;
		virtual void SetUniformMat4(const char* name, const glm::mat4& value)                          = 0;
		virtual void SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count) = 0;
		
		virtual int GetUniformLocation(const char* name) = 0;
		virtual int GetRendererID() const = 0;
	};
}