#pragma once

#include "FileManager.h"

namespace ALZ {

	class ShaderProgram
	{
	public:
		void Init(const std::string& vertPath, const std::string& fragPath);
		void Terminate();

		void Bind() const;
		void Unbind() const;

		void SetUniform1i(const char* name, const int& value);
		void SetUniform1f(const char* name, const float& value);
		void SetUniform1fs(const char* name, float * value, const int& count);
		void SetUniformVec2(const char* name, const glm::vec2& value);
		void SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count);
		void SetUniformVec3(const char* name, const glm::vec3& value);
		void SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count);
		void SetUniformVec4(const char* name, const glm::vec4& value);
		void SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count);
		void SetUniformMat4(const char* name, const glm::mat4& value);
		void SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count);

		static ShaderProgram& GetLineShader();
		static ShaderProgram& GetCircleInstancedShader();
		static ShaderProgram& GetCircleOutlineShader();
		static ShaderProgram& GetImageShader();
		static ShaderProgram& GetBackgroundShader();

	private:
		int& GetUniformLocation(const char* name);

	private:
		unsigned int m_RendererID;

		//this is to avoid getting uniform location repeatedly which is not very performant
		std::unordered_map<std::string, int> m_UniformLocationMap;
	};
}