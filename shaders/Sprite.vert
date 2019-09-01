#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 v_TexCoords;

void main() 
{
    gl_Position = u_ViewProjection * u_Model * vec4(aPos, 0.0, 1.0);
    v_TexCoords = aTexCoords;
}