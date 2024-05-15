#version 460 core

in vec2 TexCoords; 
in vec3 SkyboxCoords;
in vec3 Color;
in vec3 Normal;
in vec3 FragPosition;
in vec4 FragPositionLightSpace;
out vec4 FragColor;

uniform bool SkyboxActive;

uniform samplerCube skybox;
uniform sampler2D Texture;
uniform sampler2D Texture2;
uniform sampler2D DepthMap;

uniform bool MixEnabled;
uniform vec3 FogColor;
uniform float FogStart;
uniform float FogEnd;
uniform vec3 LightPosition;
uniform vec3 LightColor;
uniform float HazeStrength;
uniform vec3 HazeColor;
uniform float Brightness;
uniform float SetReflection;
uniform float BlurStrength;

uniform vec3 CameraPos;

float ShadowCalc(vec4 TempFragPosLightSpace, vec3 LightDirection) {
    int KernelWidth = 3; // 6x6

    vec3 Coordinates = TempFragPosLightSpace.xyz / TempFragPosLightSpace.w; // normalising
    Coordinates = Coordinates * 0.5 + 0.5; // change to [0,1] range

    float ClosestDepth = texture(DepthMap, Coordinates.xy).r; // get depth fragment
    float CurrentDepth = Coordinates.z; // get depth from fragment
    if (CurrentDepth < 0.0f || CurrentDepth > 1.0f) { // backup to prevent artifacts and random shadows
        return 0.0f; // return to skip rest of other stuff
    }

    //float Bias = 0.005f;
    float Bias = max(0.005 * (1.0 - dot(normalize(Normal), normalize(LightDirection))), 0.001);

    //float Dist = length(TempFragPosLightSpace.xyz);
    //Bias = max(Bias * (1.0f + Dist / 100.0f), 0.001f);

    float Shadow = 0.0f;
    float TexelSize = 1.0f / textureSize(DepthMap, 0).x; // assuming a square depth map

    for (int X = -KernelWidth; X <= KernelWidth; ++X) { // x and y loops to span entire depth map
        for (int Y = -KernelWidth; Y <= KernelWidth; ++Y) { 
            float PCFDepth = texture(DepthMap, Coordinates.xy + vec2(X, Y) * TexelSize).r; // get texel from step in loop
            if (CurrentDepth - Bias > PCFDepth) { // check if in shadow
                Shadow += 1.0f; // add shadowed pixel to total
            }
        }
    }

    int SampleArea = (KernelWidth * 2 + 1) * (KernelWidth * 2 + 1); // get sample area total size
    return Shadow / SampleArea; // reduce final output by how many pixels it sampled
}

void main() {
    if (SkyboxActive) {
        vec3 FlipCoords = vec3(-SkyboxCoords.x, SkyboxCoords.y, SkyboxCoords.z); // flip image
        FragColor = texture(skybox, FlipCoords); // direct texture passing
    } else {
        vec2 FlippedCoords = vec2(TexCoords.x, -TexCoords.y); // flip image because stbi loads it the wrong way
        vec3 NewTexture = texture(Texture, FlippedCoords).rgb;

        if (MixEnabled) {
            vec3 OverlayTexture = texture(Texture2, FlippedCoords).rgb;
            NewTexture = mix(NewTexture,OverlayTexture,0.4f);
        }

        // shared calculations for methods below
        vec3 Normal2 = normalize(Normal); // less clutter
        vec3 CameraDirection = normalize(CameraPos - FragPosition);
        vec3 LightDirection = normalize(LightPosition - FragPosition);  
        vec3 ReflectionDirection = reflect(-LightDirection, Normal2);

        // phong with ambient, diffuse and specular - stage 1
        vec3 Ambient = (Brightness * LightColor); // ambient
        float Diffuse = max(dot(Normal2, LightDirection), 0.0);
        vec3 MixedDiffuse = Diffuse * LightColor;
        float SpecularShine = pow(max(dot(CameraDirection, ReflectionDirection), 0.0), 80); // calculations created with help from learnopengl
        vec3 Specular = 1.0f * SpecularShine * LightColor;
        float Shadow = ShadowCalc(FragPositionLightSpace, LightDirection); // get 0-1 value through function
        vec3 Stage1 = (Ambient + (1.0 - Shadow) * (MixedDiffuse + Specular)) * NewTexture; // phong output

        //Reflection - stage 2
        vec3 CameraDirection2 = normalize(CameraPos - FragPosition);
        vec3 ReflectionVector = reflect(CameraDirection2, Normal2);
        vec3 ReflectionVector2 = vec3(ReflectionVector.x, -ReflectionVector.y, -ReflectionVector.z); // trial and error to match reflections
        vec3 ReflectedColor = texture(skybox, ReflectionVector2).rgb;
        vec3 Stage2 = mix(Stage1, ReflectedColor, SetReflection);

        // Haze - stage 3
        vec3 Stage3 = mix(Stage2, HazeColor, HazeStrength); // overlay haze on colour

        // Fog overlay - stage 4
        float FogFactor = smoothstep(FogStart, FogEnd, length(FragPosition.xyz - CameraPos)); // calculate fog strength
        vec3 Stage4 = mix(Stage3, FogColor, FogFactor);


        // output
        FragColor = vec4(Stage4,0.0f);// final convert
        //FragColor = vec4(normalize(ReflectionVector) * 0.5 + 0.5, 1.0);
    }
}