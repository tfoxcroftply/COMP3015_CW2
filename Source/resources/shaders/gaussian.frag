#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;

uniform bool HorizontalPass = false;
uniform float Intensity = 0.2f; // sort of like sigma value in gaussian

float Weights[8] = float[](0.942, 0.788, 0.586, 0.386, 0.226, 0.118, 0.054, 0.022); // gaussian values derived from gaussian function

void main()
{   
    vec3 Result = texture(ScreenTexture, TexCoords).rgb;
    vec2 TexOffset = 1.0 / textureSize(ScreenTexture, 0);

    Result *= (Weights[0] * Intensity);
    for (int i = 1; i < 8; ++i) {
        vec2 CalculatedOffset = vec2(HorizontalPass ? TexOffset.x * i : 0.0f, !HorizontalPass ? TexOffset.y * i : 0.0f);
        Result += texture(ScreenTexture, TexCoords + CalculatedOffset).rgb * (Weights[i] * Intensity);
        Result += texture(ScreenTexture, TexCoords - CalculatedOffset).rgb * (Weights[i] * Intensity);
    }
    FragColor = vec4(Result, 1.0);
}

