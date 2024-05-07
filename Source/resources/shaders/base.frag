#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;

void main()
{   
    FragColor = vec4(texture(ScreenTexture, TexCoords).rgb, 1.0);
}

