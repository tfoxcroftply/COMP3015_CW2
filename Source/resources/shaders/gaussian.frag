#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;

uniform bool HorizontalPass = false;
uniform float Intensity = 0.189f; // sort of like sigma value in gaussian. found by trial and error and is the least artifacting value. it probably is a result of a missing calculation i dont know about

float Weights[8] = float[](0.942, 0.788, 0.586, 0.386, 0.226, 0.118, 0.054, 0.022); // gaussian values derived from gaussian function

void main()
{   
    vec3 Result = texture(ScreenTexture, TexCoords).rgb; // sample pixel
    vec2 TexOffset = 1.0 / textureSize(ScreenTexture, 0);

    Result *= (Weights[0] * Intensity); // get center pixel
    for (int i = 1; i < 8; ++i) { // for every kernel loop
        vec2 CalculatedOffset = vec2(HorizontalPass ? TexOffset.x * i : 0.0f, !HorizontalPass ? TexOffset.y * i : 0.0f); // sets the offset vectors depending on selected pass orientation
        Result += texture(ScreenTexture, TexCoords + CalculatedOffset).rgb * (Weights[i] * Intensity); // sample pixels along offset in both directions and depend on intensity/kernel values
        Result += texture(ScreenTexture, TexCoords - CalculatedOffset).rgb * (Weights[i] * Intensity);
    }
    FragColor = vec4(Result, 1.0);
}

