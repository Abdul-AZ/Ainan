#version 330 core
layout(location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 FragPos;

void main() 
{
	FragPos = vec2(model * vec4(aPos / 5000, 0.0, 1.0));
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
