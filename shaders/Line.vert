#version 420 core
layout(location = 0) in vec2 aPos;

#include <common/SceneData.glsli>

void main()
{
	gl_Position = u_ViewProjection * vec4(aPos.x, aPos.y, 0.0 , 1.0);
}