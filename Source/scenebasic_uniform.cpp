
#include "scenebasic_uniform.h"
#include "BoatControls.h"
#include "ObjectGen.h"
#include "Game.h"

#define GLT_IMPLEMENTATION
#include "gltext.h"

#include <iomanip>

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

unsigned int DepthMapFramebuffer;
unsigned int DepthMapFramebufferTexture;
unsigned int DepthMapRenderbuffer;

unsigned int SceneBuffer = 0; // index positions
unsigned int BloomBuffer = 1;
unsigned int BloomBuffer2 = 2;
unsigned int BlurBuffer1 = 3;
unsigned int BlurBuffer2 = 4;

ModelData FramebufferDisplay;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}

void mouse_callback(GLFWwindow* Window, double X, double Y) { 
    boat.UpdateMouse(X, Y); // Send data to the camera for processing
}

void SceneBasic_Uniform::initScene()
{
    compile();
    prog.printActiveUniforms();
    gaussian.printActiveUniforms();

    mainWindow = glfwGetCurrentContext(); // Get the window location. Had to be called here since the lab libraries didn't seem to set it anywhere.
    boat.Window = mainWindow;

    glfwSetCursorPosCallback(mainWindow, mouse_callback); // Define mouse callback function
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
    sea.Transformation = mat4(1.0f);
    sea.Transformation = glm::rotate(sea.Transformation, glm::radians(180.0f), vec3(0.0f, 0.0f, 0.0f));
    sea.Transformation = glm::scale(mat4(1.0f), vec3(40.0f, 1.0f, 40.0f));

    boat.Init(); // boat loading logic

    if (!GameSession.Init()) { // game loading logic
        cout << "Game loading encountered an error.";
        exit(0);
    }                   



    gltInit(); // gltext library init
    Timer = gltCreateText();

    // frame buffer object
    int FrameBuffCount = sizeof(Framebuffers) / sizeof(unsigned int); // Get count

    glGenFramebuffers(FrameBuffCount, Framebuffers); // Generate buffers
    glGenTextures(FrameBuffCount, FramebufferTextures);
    glGenRenderbuffers(FrameBuffCount, Renderbuffers);

    for (unsigned int i = 0; i < FrameBuffCount; i++) { // loop for every buffer required
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

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // render buffer needed for main scene draw, but mostly unused for others
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Renderbuffers[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // depth map for shadowmap
    glGenFramebuffers(1, &DepthMapFramebuffer);
    glGenTextures(1, &DepthMapFramebufferTexture);
    //glGenRenderbuffers(1, &DepthMapRenderbuffer);

    GLfloat BorderColor[] = {1.0, 1.0, 1.0, 1.0}; // black to ensure out of frame doesnt count as a close texture

    glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); // 1024 resolution for depth map
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMapFramebufferTexture, 0); // depth parameters for correct depth detection
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cout << "Frame buffer not complete";
        exit(EXIT_FAILURE);
    }

    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthMapRenderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    FramebufferDisplay = FrameRectangle(); // generate screen quad

    prog.use();
    prog.setUniform("skybox", 0); // i found that explicit linking is required on nvidia opengl api, but seems to work without on intel
    prog.setUniform("Texture", 1); // texture linking done for each shader
    prog.setUniform("Texture2", 2);
    prog.setUniform("DepthMap", 3);

    gaussian.use(); 
    gaussian.setUniform("ScreenTexture", 0);

    bloom.use();
    bloom.setUniform("ScreenTexture", 0);
    bloom.setUniform("BaseTexture", 1);

    depth.use();
    depth.setUniform("DepthMap", 0);

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    //GameSession.LoadEntries(); for debugging
    //GameSession.SaveEntry(23023);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
}

void SceneBasic_Uniform::compile() // Provided by labs
{
	try {
		prog.compileShader("resources/shaders/shader.vert"); // Shader loading and linking
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

        depth.compileShader("resources/shaders/depth.vert");
        //depth.compileShader("resources/shaders/depth.frag");
        depth.link();
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

    for (unsigned int i = 0; i < Intensity; i++) { // Blurring logic
        bool Horizontal = i % 2 == 0 ? true : false; // Switch orientation for each pass
        shader.setUniform("HorizontalPass", Horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[StartBuffer]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void GenerateNodes(std::vector<glm::vec3> Nodes, GLSLProgram& prog, bool IsBoost) { // node generating logic, reusable for different types
    for (unsigned int i = 0; i < Nodes.size(); i++) {
        mat4 NodeBase = mat4(1.0f);
        NodeBase[3][0] = Nodes[i].x; // extract node positions and add to blank mat4
        NodeBase[3][1] = Nodes[i].y;
        NodeBase[3][2] = Nodes[i].z;

        float RotationDeg = fmod(glfwGetTime() * (360.0f / 5.0f), 360.0f); // movement logic
        NodeBase = glm::rotate(NodeBase, glm::radians(RotationDeg), vec3(0.0f, 1.0f, 0.0f));

        glActiveTexture(GL_TEXTURE1); // First texture always used
        if (IsBoost) {
            glBindTexture(GL_TEXTURE_2D, GameSession.NodeBoostTexture); // Blue texture for boost
        } else {
            glBindTexture(GL_TEXTURE_2D, (i == 0) ? GameSession.NodeNextTexture : GameSession.NodeTexture); // If first node in sequence, color green else red
        }
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture); // needs to be bound for shadows to draw over textures, this is repeated for other models
        prog.setUniform("ModelIn", NodeBase);
        GameSession.NodeModel->render();
    }
}

void RenderScene(GLSLProgram& prog) {


    // Boat render
    prog.setUniform("ModelIn", boat.GetBoatMatrix());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, boatTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture);
    boatMesh->render();

    // Sea
    mat4 SeaTranslated = glm::translate(sea.Transformation, vec3(sin(glfwGetTime()) * 0.004f, 0.0f, (sin(glfwGetTime()) * 0.004f) + 0.2f)); // bobbing movement
    prog.setUniform("ModelIn", SeaTranslated);
    prog.setUniform("MixEnabled", true);
    glActiveTexture(GL_TEXTURE1); // First texture
    glBindTexture(GL_TEXTURE_2D, seaTexture);
    glActiveTexture(GL_TEXTURE2); // Overlay texture
    glBindTexture(GL_TEXTURE_2D, seaTexture2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture);
    seaMesh->render();
    prog.setUniform("MixEnabled", false);

    // Game logic
    GameSession.UpdatePlayerPosition(boat.GetBoatPosition()); // Game session logic to track player-node position

    GenerateNodes(GameSession.GetActiveNodes(), prog, false); // generate standard nodes
    GenerateNodes(GameSession.GetActiveBoosts(), prog, true); // generate boost nodes in blue

}

void SceneBasic_Uniform::render() // Render loop
{
    prog.use(); // return original shader as gltext changes it
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // ## TIMING ##
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // ## CAMERA AND UNIFORMS ##
    if (GameSession.RequestedBoatPos != glm::vec3(0.0f, 0.0f, 0.0f)) { // If boat position is changed, then send it to the boat - has to be done this way as i cant seem to make a boat reference in other headers
        glm::vec3 Pos = GameSession.RequestedBoatPos;
        boat.Init(); // repurposed to reposition boat
        boat.SetPosition(Pos);
        GameSession.RequestedBoatPos = glm::vec3(0.0f, 0.0f, 0.0f); // reset to prevent constant loops
    }

    if (GameSession.LastBoostTime != 0) { // Same thing here, only way to transfer between headers it seems
        boat.LastBoostTime = GameSession.LastBoostTime;
        GameSession.LastBoostTime = 0;
    }

    boat.Update(deltaTime); // Delta time is used to not have abnormal movement depending on frame time
    prog.setUniform("BlurStrength", BlurIntensity);
    prog.setUniform("ModelIn", mat4(1.0f));
    prog.setUniform("MixEnabled", false); // Disable texture mixing to not cause issues 
    CameraData CamData = boat.GetCameraData(deltaTime);


    // ## SHADOWMAP RENDER ##
    depth.use();
    glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFramebuffer); // set target as depth map
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, 1024, 1024); // Depth map settings and MVP generation
    glm::mat4 LightProj = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, -12.0f, 12.0f); // tight view to ensure quality is best, also ortho to simulate light far away
    glm::vec3 NewLightPos = boat.GetBoatPosition() + (glm::normalize(LightPosition) * 0.3f); // Always pin it around the boat
    glm::mat4 LightView = glm::lookAt(NewLightPos, boat.GetBoatPosition() - glm::vec3(0.5f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    depth.setUniform("LightMatrix", LightProj * LightView); // Send MVP to shader to generate the scene from this point of view
    
    glEnable(GL_CULL_FACE); // Change cull settings to reduce "peter panning"
    glCullFace(GL_FRONT);
    RenderScene(depth); // Render the scene but only to the depth buffer
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    // ## MAIN RENDER ##

    if (!DepthDebug) {
        prog.use(); // Use main shader

        glViewport(0, 0, width, height);
        prog.setUniform("CameraPos", CamData.CameraPosition); // Send camera position for lighting calculations
        prog.setUniform("ViewIn", CamData.ViewMatrix); // Send MVP base to shader, but leave models to be set per model
        prog.setUniform("ProjectionIn", boat.Projection);

        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[SceneBuffer]); // Set the target to scene buffer, the main target
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // And clear it just in case

        // Skybox - Separated to avoid shadowmap collisions
        if (SkyboxEnabled) { // Skybox enabled is a debug setting now
            glDisable(GL_DEPTH_TEST);
            prog.setUniform("SkyboxActive", true); // Fragment shader setting to skybox
            glBindVertexArray(skybox.Data.VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.Data.TextureID);
            glDrawArrays(GL_TRIANGLES, 0, skybox.Data.ArraySize);
            glEnable(GL_DEPTH_TEST);
            prog.setUniform("SkyboxActive", false);
        }

        glActiveTexture(GL_TEXTURE3); // Send the generated depth map to the shader
        glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture);
        prog.setUniform("LightMatrix", LightProj * LightView); // and associated light position values

        RenderScene(prog); // Render the scene as usual, with the normal shader

        // ## POST PROCESSING ##
        glDisable(GL_DEPTH_TEST); // This was required for the screen frame to be drawn


        bloom.use();
        bloom.setUniform("Mix", false); // select non mix mode to generate bloom buffer
        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[BloomBuffer]);
        glBindVertexArray(FramebufferDisplay.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[SceneBuffer]); // render it to scene buffer for later
        glDrawArrays(GL_TRIANGLES, 0, 6);

        gaussian.use();
        Blur(6, gaussian, BloomBuffer); // use the blurring function to blur the bloom buffer to make it smooth

        bloom.use();
        bloom.setUniform("Mix", true); // Mix the render scene buffer with the new bloom contents
        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffers[SceneBuffer]);
        glBindVertexArray(FramebufferDisplay.VAO);
        glActiveTexture(GL_TEXTURE0); // set both texture units this time
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[BloomBuffer]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[SceneBuffer]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        int Cycles = boat.GetBlurCycles(); // Gets the number of blur cycles to use, usually after stage completions
        if (Cycles > 0 or GameSession.IsFinished() or BlurDebug) {
            gaussian.use();
            Blur(4, gaussian, SceneBuffer); // Static number, it was intended to use dynamic numbers at first
        }

        base.use();
        base.setUniform("DepthMode", false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(FramebufferDisplay.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferTextures[BloomDebug == true ? BloomBuffer : SceneBuffer]); // Select bloom or scene buffer based on bloom debug setting
        glDrawArrays(GL_TRIANGLES, 0, 6);
    } else {
        base.use(); // If depth testing enabled, only use this to display depth map
        base.setUniform("DepthMode", true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(FramebufferDisplay.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DepthMapFramebufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
    
    if (GameSession.MenuOption == "Start") { 
        gltSetText(Timer, "Find the first checkpoint to begin the game!"); // Display start message
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2DAligned(Timer, this->width / 2, this->height - 50, 1.5f, GLT_CENTER, GLT_BOTTOM);
        gltEndDraw();

    } else if (GameSession.MenuOption == "Running") { // Display game related messages
        std::string NodeText = "Level: " + std::to_string(GameSession.ActiveLevel) + "/" + std::to_string(GameSession.GetLevelCount());
        gltSetText(Timer, NodeText.c_str());
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2DAligned(Timer, this->width - 5, this->height - 65, 1.5f, GLT_RIGHT, GLT_BOTTOM);
        gltEndDraw();

        std::string LevelText = "Nodes: " + std::to_string(GameSession.LastNodeTotal - GameSession.LastNodeCount) + "/" + std::to_string(GameSession.LastNodeTotal); // Display time
        gltSetText(Timer, LevelText.c_str());
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2DAligned(Timer, this->width - 5, this->height - 35, 1.5f, GLT_RIGHT, GLT_BOTTOM);
        gltEndDraw();

        std::ostringstream TimeText;
        TimeText << "Time: " << std::fixed << std::setprecision(3) << float(GameSession.GetTime()) / 1000.0f; // format to 3 decimal places
        gltSetText(Timer, TimeText.str().c_str());
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2DAligned(Timer, this->width - 5, this->height - 5, 1.5f, GLT_RIGHT, GLT_BOTTOM);
        gltEndDraw();

    } else if (GameSession.MenuOption == "Finish") { // Display finish messages
        gltSetText(Timer, "Finish!");
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2DAligned(Timer, this->width / 2, 150, 2.5f, GLT_CENTER, GLT_TOP);
        gltEndDraw();

        GameData Entries = GameSession.LoadEntries(); // For each entry returned, make an entry and display it
        for (unsigned int i = 0; i < Entries.MillisecondsSorted.size(); i++) {
            int Entry = Entries.MillisecondsSorted[i];

            std::ostringstream EntryText;
            EntryText << i + 1 << " - " << std::fixed << std::setprecision(5) << float(Entry) / 1000.0f;

            gltSetText(Timer, EntryText.str().c_str());
            gltBeginDraw();

            if (Entry == GameSession.SaveTime) { // If it is the current session's time, highlight it green
                gltColor(0.0f, 1.0f, 0.0f, 1.0f);
            } else {
                gltColor(1.0f, 1.0f, 1.0f, 1.0f);
            }

            gltDrawText2DAligned(Timer, this->width / 2, 200 + (i * 30), 1.5f, GLT_CENTER, GLT_TOP);
            gltEndDraw();
        }

    }

    glBindVertexArray(0); // Unbinding

    //glBindVertexArray(0);

    //Timing
    while (glfwGetTime() - currentFrame < 1.0f / FrameRate) {}
}

void SceneBasic_Uniform::resize(int w, int h)
{
    boat.Projection = glm::perspective(glm::radians(float(FOV)), (float)w / (float)h, 0.01f, 100.0f);
    glViewport(0,0,w,h);
}


