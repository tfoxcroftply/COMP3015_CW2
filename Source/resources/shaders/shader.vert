#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoords;

out vec3 SkyboxCoords;
out vec2 TexCoords;
out vec3 Color;
out vec3 Normal;
out vec3 FragPosition;

uniform mat4 ModelIn;
uniform mat4 ViewIn;
uniform mat4 ProjectionIn;
uniform bool SkyboxActive;

void main() {
    if (!SkyboxActive) {
        Color = vec3(0.5, 0.5, 0.5); // default color for if textures dont load
        Normal = mat3(transpose(inverse(ModelIn))) * VertexNormal; 
        FragPosition = vec3(ModelIn * vec4(VertexPosition, 1.0));
        TexCoords = VertexTexCoords;
        gl_Position = ProjectionIn * ViewIn * ModelIn * vec4(VertexPosition, 1.0);
    } else {
        SkyboxCoords = VertexPosition;
        gl_Position = ProjectionIn * mat4(mat3(ViewIn)) * vec4(VertexPosition, 1.0);
    }
}
