#version 330 core
layout (location = 0) in vec3 VertexPos;
layout (location = 1) in vec2 VertexTexCoords;

uniform mat4 LightMatrix;
uniform mat4 ModelIn;

void main()
{
    gl_Position = LightMatrix * ModelIn * vec4(VertexPos, 1.0);
}  