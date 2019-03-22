#include <pch.h>
#include "ShaderProgram.h"

std::string ReadEntireTextFile(const std::string& path) 
{
	std::string file_content;

	FILE* file = fopen(path.c_str(), "r");

	fseek(file, 0, SEEK_END);
	int content_length = ftell(file);
	file_content.resize(content_length + 1);
	fseek(file, 0, SEEK_SET);

	fread((void*)file_content.data(), sizeof(char), content_length, file);

	fclose(file);

	return file_content;
}


void ShaderProgram::Init(const std::string & vertPath, const std::string & fragPath)
{
	unsigned int vertex, fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	std::string vShaderCode = ReadEntireTextFile(vertPath);
	const char* c_vShaderCode = vShaderCode.c_str();
	glShaderSource(vertex, 1, &c_vShaderCode, NULL);
	glCompileShader(vertex);


	std::string fShaderCode = ReadEntireTextFile(fragPath);
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

void ShaderProgram::Bind() const
{
	glUseProgram(m_RendererID);
}

void ShaderProgram::Unbind() const
{
	glUseProgram(0);
}

void ShaderProgram::setUniform1i(const char * name, const int& value)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform1i(location, value);
}

void ShaderProgram::setUniform1f(const char * name, const float& value)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform1f(location, value);
}

void ShaderProgram::setUniform1fs(const char * name, float * value, const int & count)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform1fv(location,count, value);
}

void ShaderProgram::setUniformVec2(const char* name, const glm::vec2& value)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform2f(location, value.x, value.y);
}

void ShaderProgram::setUniformVec4(const char* name, const glm::vec4& value) 
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void ShaderProgram::setUniformVec4s(const char * name, const glm::vec4* const value, const int & count)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniform4fv(location, count, (float*)value);
}

void ShaderProgram::setUniformMat4(const char * name, const glm::mat4 & value)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniformMat4s(const char * name, const glm::mat4* const value, const int & count)
{
	Bind();
	int location = glGetUniformLocation(m_RendererID, name);
	glUniformMatrix4fv(location, count, GL_FALSE, (float*)value);
}
