#version 330 core
out vec4 FragColor;

layout (std140) uniform ObjectColor
{
	uniform vec4 u_Color;
};

void main()
{
	FragColor = u_Color;
}