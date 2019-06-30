#include <pch.h>
#include <glad/glad.h>

#include "OpenGLShaderProgram.h"

namespace ALZ {
	namespace OpenGL {

		OpenGLShaderProgram::OpenGLShaderProgram(const std::string& vertPath, const std::string& fragPath)
		{
			unsigned int vertex, fragment;

			vertex = glCreateShader(GL_VERTEX_SHADER);
			std::string vShaderCode = FileManager::ReadEntireTextFile(vertPath);
			const char* c_vShaderCode = vShaderCode.c_str();
			glShaderSource(vertex, 1, &c_vShaderCode, NULL);
			glCompileShader(vertex);


			std::string fShaderCode = FileManager::ReadEntireTextFile(fragPath);
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			const char* c_fShaderCode = fShaderCode.c_str();
			glShaderSource(fragment, 1, &c_fShaderCode, NULL);
			glCompileShader(fragment);


			// shader Program
			m_RendererID = glCreateProgram();
			glAttachShader(m_RendererID, vertex);
			glAttachShader(m_RendererID, fragment);

			glLinkProgram(m_RendererID);
			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			Bind();
		}

		OpenGLShaderProgram::~OpenGLShaderProgram()
		{
			glDeleteProgram(m_RendererID);
		}

		void OpenGLShaderProgram::Bind() const
		{
			glUseProgram(m_RendererID);
		}

		void OpenGLShaderProgram::Unbind() const
		{
			glUseProgram(0);
		}

		void OpenGLShaderProgram::SetUniform1i(const char* name, const int& value)
		{
			Bind();
			glUniform1i(GetUniformLocation(name), value);
		}

		void OpenGLShaderProgram::SetUniform1f(const char* name, const float& value)
		{
			Bind();
			glUniform1f(GetUniformLocation(name), value);
		}

		void OpenGLShaderProgram::SetUniform1fs(const char* name, float* value, const int& count)
		{
			Bind();
			glUniform1fv(GetUniformLocation(name), count, value);
		}

		void OpenGLShaderProgram::SetUniformVec2(const char* name, const glm::vec2& value)
		{
			Bind();
			glUniform2f(GetUniformLocation(name), value.x, value.y);
		}

		void OpenGLShaderProgram::SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count)
		{
			Bind();
			glUniform2fv(GetUniformLocation(name), count, (float*)value);
		}

		void OpenGLShaderProgram::SetUniformVec3(const char* name, const glm::vec3& value)
		{
			Bind();
			glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
		}

		void OpenGLShaderProgram::SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count)
		{
			Bind();
			glUniform3fv(GetUniformLocation(name), count, (float*)value);
		}

		void OpenGLShaderProgram::SetUniformVec4(const char* name, const glm::vec4& value)
		{
			Bind();
			glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
		}

		void OpenGLShaderProgram::SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count)
		{
			Bind();
			glUniform4fv(GetUniformLocation(name), count, (float*)value);
		}

		void OpenGLShaderProgram::SetUniformMat4(const char* name, const glm::mat4& value)
		{
			Bind();
			glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
		}

		void OpenGLShaderProgram::SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count)
		{
			Bind();
			glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, (float*)value);
		}

		int OpenGLShaderProgram::GetUniformLocation(const char* name)
		{
			if (m_UniformLocationMap.find(name) != m_UniformLocationMap.end())
				return m_UniformLocationMap[name];

			m_UniformLocationMap[name] = glGetUniformLocation(m_RendererID, name);
			return m_UniformLocationMap[name];
		}
	}
}