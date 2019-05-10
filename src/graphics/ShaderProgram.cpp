#include <pch.h>
#include "ShaderProgram.h"

namespace ALZ {

	static ShaderProgram LineShader;
	static bool LineShaderInitilized = false;

	static ShaderProgram CircleInstancedShader;
	static bool CircleInstancedShaderInitilized = false;

	static ShaderProgram CircleOutlineShader;
	static bool CircleOutlineShaderInitilized = false;

	static ShaderProgram ImageShader;
	static bool ImageShaderInitilized = false;

	static ShaderProgram BackgroundShader;
	static bool BackgroundShaderInitilized = false;

	void ShaderProgram::Init(const std::string & vertPath, const std::string & fragPath)
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

	void ShaderProgram::Terminate()
	{
		glDeleteProgram(m_RendererID);
	}

	void ShaderProgram::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void ShaderProgram::Unbind() const
	{
		glUseProgram(0);
	}

	void ShaderProgram::SetUniform1i(const char * name, const int& value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void ShaderProgram::SetUniform1f(const char * name, const float& value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform1f(location, value);
	}

	void ShaderProgram::SetUniform1fs(const char * name, float * value, const int & count)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform1fv(location, count, value);
	}

	void ShaderProgram::SetUniformVec2(const char* name, const glm::vec2& value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform2f(location, value.x, value.y);
	}

	void ShaderProgram::SetUniformVec2s(const char * name, const glm::vec2 * const value, const int & count)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform2fv(location, count, (float*)value);
	}

	void ShaderProgram::SetUniformVec3(const char * name, const glm::vec3& value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform3f(location, value.x, value.y, value.z);
	}

	void ShaderProgram::SetUniformVec3s(const char * name, const glm::vec3 * const value, const int & count)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform3fv(location, count, (float*)value);
	}

	void ShaderProgram::SetUniformVec4(const char* name, const glm::vec4& value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void ShaderProgram::SetUniformVec4s(const char * name, const glm::vec4* const value, const int & count)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniform4fv(location, count, (float*)value);
	}

	void ShaderProgram::SetUniformMat4(const char * name, const glm::mat4 & value)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void ShaderProgram::SetUniformMat4s(const char * name, const glm::mat4* const value, const int & count)
	{
		Bind();
		int& location = GetUniformLocation(name);
		glUniformMatrix4fv(location, count, GL_FALSE, (float*)value);
	}

	ShaderProgram& ShaderProgram::GetLineShader()
	{
		if (!LineShaderInitilized)
		{
			LineShader.Init("shaders/Line.vert", "shaders/Line.frag");
			LineShaderInitilized = true;
		}

		return LineShader;
	}

	ShaderProgram& ShaderProgram::GetCircleInstancedShader()
	{
		if (!CircleInstancedShaderInitilized)
		{
			CircleInstancedShader.Init("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag");
			CircleInstancedShaderInitilized = true;
		}

		return CircleInstancedShader;
	}

	ShaderProgram& ShaderProgram::GetCircleOutlineShader()
	{
		if (!CircleOutlineShaderInitilized)
		{
			CircleOutlineShader.Init("shaders/CircleOutline.vert", "shaders/CircleOutline.frag");
			CircleOutlineShaderInitilized = true;
		}

		return CircleOutlineShader;
	}

	ShaderProgram& ShaderProgram::GetImageShader()
	{
		if (!ImageShaderInitilized)
		{
			ImageShader.Init("shaders/Image.vert", "shaders/Image.frag");
			ImageShaderInitilized = true;
		}

		return ImageShader;
	}

	ShaderProgram& ShaderProgram::GetBackgroundShader()
	{
		if (!BackgroundShaderInitilized)
		{
			BackgroundShader.Init("shaders/Background.vert", "shaders/Background.frag");
			BackgroundShaderInitilized = true;
		}

		return BackgroundShader;
	}

	int& ShaderProgram::GetUniformLocation(const char * name)
	{
		if (m_UniformLocationMap.find(name) != m_UniformLocationMap.end())
			return m_UniformLocationMap[name];

		m_UniformLocationMap[name] = glGetUniformLocation(m_RendererID, name);
		return m_UniformLocationMap[name];
	}
}