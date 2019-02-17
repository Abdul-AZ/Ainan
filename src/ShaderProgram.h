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

	void setUniformVec2(const char* name, const glm::vec2& value);
	void setUniformVec4(const char* name, const glm::vec4& value);
	void setUniformMat4(const char* name, const glm::mat4& value);

private:
	unsigned int m_RendererID;
};