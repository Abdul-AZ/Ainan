#version 420 core
#include <common/SceneData.glsli>

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(location = 0) out vec2 TexCoords;

layout (std140, binding = 1) uniform ObjectTransform
{
    uniform mat4 u_Model;
};

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_ViewProjection * u_Model * vec4(aPos, 1.0);
}