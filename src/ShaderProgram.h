#pragma once

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram 
{
public:
	void Init(const std::string& vertPath, const std::string& fragPath);

	void Bind() const;
	void Unbind() const;

	void setUniform1i(const char* name, int value);
	void setUniformVec2(const char* name, const glm::vec2& value);
	void setUniformVec4(const char* name, const glm::vec4& value);
	void setUniformVec4s(const char* name, const glm::vec4* const value, const int& count);
	void setUniformMat4(const char* name, const glm::mat4& value);
	void setUniformMat4s(const char* name, const glm::mat4* const value, const int& count);

private:
	unsigned int m_RendererID;
};