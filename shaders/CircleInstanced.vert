#version 330 core
layout(location = 0) in vec2 aPos;

uniform vec4 colorArr[40];
uniform mat4 model[40];
uniform mat4 projection;

out vec4 color;

void main()
{
    gl_Position = projection * model[gl_InstanceID] * vec4(aPos.x, aPos.y, 0.0, 1.0);
	color = colorArr[gl_InstanceID];
}