#version 410 core

layout (location = 0) in vec3 inPosition;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 worldPosition;

void main()
{
    vec4 worldPos = modelMat * vec4(inPosition, 1);
    gl_Position = projMat * viewMat * worldPos;
    worldPosition = worldPos.xyz;
}