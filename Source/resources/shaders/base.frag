#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;

uniform bool DepthMode = false;

void main()
{   
    if (!DepthMode) {
        FragColor = vec4(texture(ScreenTexture, TexCoords).rgb, 1.0);
    } else {
        float Depth = texture(ScreenTexture, TexCoords).r;
        FragColor = vec4(vec3(Depth), 1.0);
    }
}

