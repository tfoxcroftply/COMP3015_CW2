
#include "scenebasic_uniform.h"
#include "BoatControls.h"
#include "ObjectGen.h"
#include "Game.h"

#define GLT_IMPLEMENTATION
#include "gltext.h"

using std::string;
using std::cerr;
using std::endl;
using std::cout;
using glm::vec3;
using glm::mat4;
using glm::scale;

// Would have defined it in the header however it seemed to break STBI
Boat boat;
GLFWwindow* mainWindow;
Model object;
Model skybox;
Model sea;
std::unique_ptr<ObjMesh> boatMesh;
std::unique_ptr<ObjMesh> seaMesh;
int boatTexture;
int seaTexture;
int seaTexture2;
Game GameSession;
GLTtext* Timer;

unsigned int Framebuffers[5];
unsigned int FramebufferTextures[5];
unsigned int Renderbuffers[5];

unsigned int SceneBuffer = 0;
unsigned int BloomBuffer = 1;
unsigned int BloomBuffer2 = 2;
unsigned int BlurBuffer1 = 3;
unsigned int BlurBuffer2 = 4;

ModelData FramebufferDisplay;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}

//void mouse_callback(GLFWwindow* Window, double X, double Y) {
  //  camera.MouseInput(X, Y); // Send data to the camera for processing
//}

void SceneBasic_Uniform::initScene()
{
    compile();
    prog.printActiveUniforms();
    gaussian.printActiveUniforms();

    mainWindow = glfwGetCurrentContext(); // Get the window location. Had to be called here since the lab libraries didn't seem to set it anywhere.
    boat.Window = mainWindow;

    //glfwSetCursorPosCallback(mainWindow, mouse_callback); // Define mouse callback function
    glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    prog.setUniform("FogColor", FogColor); // Pass all lighting variables to the fragment shader
    prog.setUniform("FogStart", FogStartDist);
    prog.setUniform("FogEnd", FogEndDist);
    prog.setUniform("HazeStrength", HazeStrength);
    prog.setUniform("HazeColor", HazeColor);
    prog.setUniform("Brightness", Brightness);
    prog.setUniform("LightPosition", LightPosition);
    prog.setUniform("LightColor", LightColor);
    prog.setUniform("SetReflection", Reflectance);

    prog.setUniform("skybox", 0); // i found that explicit linking is required on nvidia opengl api, but seems to work without on intel
    prog.setUniform("Texture", 1);
    prog.setUniform("Texture2", 2);


    // Object gen
    //object.Data = GenerateSquare();
    //object.Transformation = mat4(1.0f);
    ///object.Transformation = glm::rotate(object.Transformation, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

    // Skybox object
    skybox.Data = GenerateSkybox(); // Grab skybox data from ObjectGen
    skybox.Transformation = mat4(1.0f);
    //skybox.Transformation = scale(skybox.Transformation, vec3(1.0f, 1.0f, 999.0f));

    // Boat object
    boatMesh = ObjMesh::load("resources/models/boat.obj");
    boatTexture = LoadTexture("resources/textures/boat.png");

    // Sea object
    seaMesh = ObjMesh::load("resources/models/sea.obj");
    seaTexture = LoadTexture("resources/textures/sea.png");
    seaTexture2 = LoadTexture("resources/textures/seaoverlay.png");
    sea.Transformation = glm::rotate(sea.Transformation, glm::radians(180.0f), vec3(0.0f, 0.0f, 0.0f));
    sea.Transformation = glm::scale(mat4(1.0f), vec3(40.0f, 1.0f, 40.0f));

    if (!GameSession.Init()) {
        cout << "Game loading encountered an error.";
        exit(0);
    }                   

    boat.Init();

    gltInit();
    Timer = gltCreateText();

    // frame buffer object
    int FrameBuffCount = sizeof(Framebuffers) / sizeof(unsigned int);

    glGenFramebuffers(FrameBuffCount, Framebuffers);
    glGenTextures(FrameBuffCount, FramebufferTextures);
    glGenRenderbuffers(FrameBuffCount, Renderbuffers);

    for (unsigned int i = 0; i < FrameBuffCount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[i]);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, Renderbuffers[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferTextures[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cout << "Frame buffer not complete";
            exit(EXIT_FAILURE);
        }

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Renderbuffers[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    FramebufferDisplay = FrameRectangle();

    gaussian.use();
    gaussian.setUniform("ScreenTexture", 0);

    bloom.use();
    bloom.setUniform("ScreenTexture", 0);
    bloom.setUniform("BaseTexture", 1);

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    GameSession.LoadEntries();
    GameSession.SaveEntry(23023);
}

void SceneBasic_Uniform::compile() // Provided by labs
{
	try {
		prog.compileShader("resources/shaders/shader.vert");
		prog.compileShader("resources/shaders/shader.frag");
		prog.link();
		prog.use();

        gaussian.compileShader("resources/shaders/gaussian.vert");
        gaussian.compileShader("resources/shaders/gaussian.frag");
        gaussian.link();

        bloom.compileShader("resources/shaders/bloom.vert");
        bloom.compileShader("resources/shaders/bloom.frag");
        bloom.link();

        base.compileShader("resources/shaders/base.vert");
        base.compileShader("resources/shaders/base.frag");
        base.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    // unused
}

void Blur(int Intensity, GLSLProgram& shader, int StartBuffer) {
    //glBindFramebuffer(GL_FRAMEBUFFER, SceneBuffer);

    for (unsigned int i = 0; i < Intensity; i++) {
        bool Horizontal = i % 2 == 0 ? true : false;
        shader.setUniform("HorizontalPass", Horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[StartBuffer]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void SceneBasic_Uniform::render() // Render loop
{

    prog.use(); // return original shader as gltext changes it
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[SceneBuffer]);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Scene clearing/refreshing

    //Timing
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //Camera
    boat.Update(deltaTime); // Delta time is used to not have abnormal movement depending on frame time

    prog.setUniform("BlurStrength", BlurIntensity);

    //Others
    prog.setUniform("ModelIn", mat4(1.0f));
    prog.setUniform("MixEnabled", false); // Disable texture mixing to not cause issues 

    CameraData CamData = boat.GetCameraData(deltaTime);

    prog.setUniform("CameraPos", CamData.CameraPosition); // Send camera position for lighting calculations
    prog.setUniform("ViewIn", CamData.ViewMatrix); // Send MVP base to shader, but leave models to be set per model
    prog.setUniform("ProjectionIn", boat.Projection);

    //Skybox
    glDisable(GL_DEPTH_TEST);
    prog.setUniform("SkyboxActive", true); // Fragment shader setting to skybox
    glBindVertexArray(skybox.Data.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.Data.TextureID);
    glDrawArrays(GL_TRIANGLES, 0, skybox.Data.ArraySize);
    glEnable(GL_DEPTH_TEST);
    prog.setUniform("SkyboxActive", false);

    // Boat render
    prog.setUniform("ModelIn", boat.GetBoatMatrix());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, boatTexture);
    boatMesh->render();

    // Sea
    mat4 SeaTranslated = glm::translate(sea.Transformation, vec3(sin(glfwGetTime()) * 0.004f, 0.0f, (sin(glfwGetTime()) * 0.004f) + 0.2f));
    prog.setUniform("ModelIn", SeaTranslated);
    prog.setUniform("MixEnabled", true);
    glActiveTexture(GL_TEXTURE1); // First texture
    glBindTexture(GL_TEXTURE_2D, seaTexture);
    glActiveTexture(GL_TEXTURE2); // Overlay texture
    glBindTexture(GL_TEXTURE_2D, seaTexture2);
    seaMesh->render();
    prog.setUniform("MixEnabled", false);

    // Game logic
    GameSession.UpdatePlayerPosition(boat.GetBoatPosition());
    std::vector<glm::vec3> Nodes = GameSession.GetActiveNodes();
    for (unsigned int i = 0; i < Nodes.size(); i++) {
        mat4 NodeBase = mat4(1.0f);
        NodeBase[3][0] = Nodes[i].x;
        NodeBase[3][1] = Nodes[i].y;
        NodeBase[3][2] = Nodes[i].z;

        float RotationDeg = fmod(glfwGetTime() * (360.0f / 5.0f), 360.0f);
        NodeBase = glm::rotate(NodeBase, glm::radians(RotationDeg), vec3(0.0f, 1.0f, 0.0f));
        glActiveTexture(GL_TEXTURE1); // First texture
        glBindTexture(GL_TEXTURE_2D, (i == 0) ? GameSession.NodeNextTexture : GameSession.NodeTexture);
        prog.setUniform("ModelIn", NodeBase);
        GameSession.NodeModel->render();
    }


    glDisable(GL_DEPTH_TEST);

    bloom.use();
    bloom.setUniform("Mix", false);
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[BloomBuffer]);
    glBindVertexArray(FramebufferDisplay.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FramebufferTextures[SceneBuffer]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    gaussian.use();
    Blur(6, gaussian, BloomBuffer);

    bloom.use();
    bloom.setUniform("Mix", true);
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[SceneBuffer]);
    glBindVertexArray(FramebufferDisplay.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FramebufferTextures[BloomBuffer]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, FramebufferTextures[SceneBuffer]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    base.use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(FramebufferDisplay.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FramebufferTextures[BloomDebug == true ? BloomBuffer : SceneBuffer]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);


    /*gltSetText(Timer, "Hello World!");
    //gltBeginDraw();
    //gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    //gltDrawText2DAligned(Timer, this->width / 2, this->height - 50, 1.5f, GLT_CENTER, GLT_BOTTOM);
    //gltEndDraw(); */

    //glBindVertexArray(0);

    //Timing
    while (glfwGetTime() - currentFrame < 1.0f / FrameRate) {}
}

void SceneBasic_Uniform::resize(int w, int h)
{
    boat.Projection = glm::perspective(glm::radians(float(FOV)), (float)w / (float)h, 0.01f, 100.0f);
    glViewport(0,0,w,h);
}


