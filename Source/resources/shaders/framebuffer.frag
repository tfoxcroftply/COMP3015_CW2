#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;
uniform bool HorizontalPass = false;
uniform bool BlurEnabled = false;

float Weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216); // gaussian values

void main()
{   
    vec3 Result = texture(ScreenTexture, TexCoords).rgb;
    if (BlurEnabled) {
        vec2 TexOffset = 1.0 / textureSize(ScreenTexture, 0);
        Result *= Weights[0];
        for(int i = 1; i < 5; ++i) {
            vec2 CalculatedOffset = vec2(HorizontalPass ? TexOffset.x * i : 0.0f, !HorizontalPass ? TexOffset.y * i : 0.0f);
            Result += texture(ScreenTexture, TexCoords + CalculatedOffset).rgb * Weights[i];
            Result += texture(ScreenTexture, TexCoords - CalculatedOffset).rgb * Weights[i];
        }
    }
    FragColor = vec4(Result, 1.0);
}

