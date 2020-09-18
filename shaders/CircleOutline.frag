#version 420 core

layout(location = 0) out vec4 FragColor;

layout (std140, binding = 2) uniform ObjectColor
{
	uniform vec4 u_Color;
};

void main()
{
	FragColor = u_Color;
}