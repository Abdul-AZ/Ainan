#version 420 core
layout (location = 0) in vec3 aPos;

#include <common/SceneData.glsli>

layout(location = 0) out vec3 TexCoords;

layout (std140, binding = 1) uniform SkyboxTransform
{
    uniform mat4 u_SkyboxViewProjection;
};

void main()
{
    TexCoords = aPos;
    gl_Position = u_SkyboxViewProjection * vec4(aPos, 1.0);
}  