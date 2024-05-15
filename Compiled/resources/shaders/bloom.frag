#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D ScreenTexture;
uniform sampler2D BaseTexture;

uniform bool Mix = false;
uniform float Exposure = 0.5f;

void main()
{   
    vec3 Result = vec3(0.0f);
    if (!Mix) { // mode selection. 
        Result = texture(ScreenTexture, TexCoords).rgb; // get pixel colour
        float Brightness = dot(Result, vec3(0.21, 0.71, 0.07)); // lumaince coefficients

        if (Brightness >= 0.96f && Brightness < 1.0f) {
            float fadeFactor = (Brightness - 0.96f) / 0.04f; // fading function to prevent sharp cutoffs
            Result *= fadeFactor;
        } else {
            Result = vec3(0.0f); // if not in range, just discard
        }
    } else { // Mix textures from both sampler2ds
        Result = texture(BaseTexture, TexCoords).rgb;
        vec3 BloomResult = texture(ScreenTexture, TexCoords).rgb;
        Result += BloomResult * Exposure; // overlay bloom at a set exposure level
    }

    FragColor = vec4(Result, 1.0);
}