#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/glutils.h"
#include "helper/glslprogram.h"
#include "helper/objmesh.h"

#include <GLFW/glfw3.h>

class SceneBasic_Uniform : public Scene
{
private:

    GLuint vaoHandle;
    GLSLProgram prog;
    GLSLProgram gaussian;
    GLSLProgram bloom;
    GLSLProgram base;
    GLSLProgram depth;

    float angle;

    const int FrameRate = 144;
    const int FOV = 70;

    const glm::vec3 FogColor = glm::vec3(0.91f, 0.92f, 0.93f);
    const float FogStartDist = 5.0f;
    const float FogEndDist = 50.0f;
    const float HazeStrength = 0.07f; // Sort of like a scene tint
    const glm::vec3 HazeColor = glm::vec3(0.9f, 1.0f, 1.0f); // very slightly blue
    const glm::vec3 LightPosition = glm::vec3(1000.0f, 1000.0f, -1000.0f);
    const glm::vec3 LightColor = glm::vec3(0.7f, 0.7f, 0.7f);

    const float Brightness = 1.0f;
    const float Reflectance = 0.4f;

    float MovementDistance = 1.0f; //boat stuff
    float SpeedMultiplier = 3.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float startTime = glfwGetTime();

    float BlurIntensity = 0.0f;

    bool BloomDebug = false;
    bool MovementEnabled = true;
    bool SkyboxEnabled = true;
    bool DepthDebug = false;

    void compile();

public:
    SceneBasic_Uniform();
    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
