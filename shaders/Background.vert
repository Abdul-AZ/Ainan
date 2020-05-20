#version 330 core
layout(location = 0) in vec2 aPos;

layout (std140) uniform FrameData
{
	mat4 u_ViewProjection;
};

uniform mat4 u_Model;

out vec2 FragPos;

void main() 
{
	vec4 pos = u_Model * vec4(aPos.x,aPos.y, 0.0, 1.0);
	FragPos = pos.xy;
	gl_Position = u_ViewProjection * u_Model * vec4(aPos.xy, 0.0, 1.0);
}
