#version 420 core
layout(location = 0) in vec2 aPos;

layout (std140, binding = 0) uniform FrameData
{
	mat4 u_ViewProjection;
};

void main()
{
	gl_Position = u_ViewProjection * vec4(aPos.x, aPos.y, 0.0 , 1.0);
}