#version 420 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

#include <common/SceneData.glsli>

layout (std140, binding = 1) uniform ObjectTransform
{
    uniform vec2 u_Position;
    uniform float u_AspectRatio;
    uniform float u_OpacityR;
    uniform float u_OpacityG;
};

layout(location = 0) out vec4 v_Color;

void main()
{
    vec2 projectedPos = vec2(aPos.x * u_AspectRatio, aPos.y);
    gl_Position = vec4(u_Position.x + projectedPos.x, u_Position.y + projectedPos.y, 0.0, 1.0);
    v_Color = vec4(aColor.r * u_OpacityR, aColor.g * u_OpacityG, aColor.ba);
}