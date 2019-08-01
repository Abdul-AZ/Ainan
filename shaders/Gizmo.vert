#version 330 core
layout(location = 0) in vec2 aPos;

uniform mat4 model;

uniform mat4 u_ViewProjection;

void main()
{
    gl_Position = u_ViewProjection * model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}