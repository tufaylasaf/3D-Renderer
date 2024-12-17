#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Model.h"
#include "light.h"
#include "skybox.h"

const unsigned int width = 1600;
const unsigned int height = 900;

const unsigned int samples = 8;

std::vector<Model *> Model::models;
std::vector<Light *> Light::lights;
int Light::pointLightCount = 0;

float rectangleVertices[] =
    {
        // Coords    // texCoords
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, samples);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "tuf3D", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);

    int windowPosX = (videoMode->width - width) / 2;
    int windowPosY = (videoMode->height - height) / 2;

    glfwSetWindowPos(window, windowPosX, windowPosY);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    gladLoadGL();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, width, height);

    Shader lightShader("res/shaders/light.vert", "res/shaders/light.frag");
    Shader pbrShader("res/shaders/default.vert", "res/shaders/pbr_textured.frag");
    Shader frameBufferShader("res/shaders/framebuffer.vert", "res/shaders/framebuffer.frag");

    Shader raymarchShader("res/shaders/default.vert", "res/shaders/raymarch.frag");

    Model cube("res/models/Shapes/cube.gltf", "Cube", false);

    // Model statue("res/models/marble_bust_01_2k/marble_bust_01_2k.gltf", "res/models/marble_bust_01_2k/textures", "Statue", true);

    Light dLight("res/models/Shapes/sphere.gltf", "DLight", "Directional");
    Light pLight("res/models/Shapes/sphere.gltf", "PLight", "Point");
    Light pLight2("res/models/Shapes/sphere.gltf", "PLight2", "Point");
    Light pLight3("res/models/Shapes/sphere.gltf", "PLight3", "Point");
    Light pLight4("res/models/Shapes/sphere.gltf", "PLight4", "Point");
    Light sLight("res/models/Shapes/sphere.gltf", "SLight", "Spot");

    frameBufferShader.Activate();
    glUniform1i(glGetUniformLocation(frameBufferShader.ID, "screenTexture"), 0);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    // ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    unsigned int rectVAO, rectVBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Create Framebuffer Texture
    unsigned int framebufferTexture;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

    // Create Render Buffer Object
    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Error checking framebuffer
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer error: " << fboStatus << std::endl;

    // glfwSwapInterval(1);

    float marchSize = 0.08f;
    float lightMarchSize = 0.03f;
    float absorptionCoEff = 0.9f;
    int frameCount = 0;

    Texture blueNoiseTexture("res/textures/blueNoise.png", "2D", 1);
    blueNoiseTexture.texUnit(raymarchShader, "uBlueNoise", 1);

    Texture noiseTexture("res/textures/noise2 (1).png", "2D", 2);
    noiseTexture.texUnit(raymarchShader, "uNoise", 2);

    while (!glfwWindowShouldClose(window))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        frameCount++;

        raymarchShader.Activate();
        blueNoiseTexture.Bind();
        noiseTexture.Bind();
        glUniform2f(glGetUniformLocation(raymarchShader.ID, "iResolution"), (float)width, (float)height);
        // glUniform1i(glGetUniformLocation(raymarchShader.ID, "uFrame"), frameCount);

        float timeValue = glfwGetTime();
        glUniform1f(glGetUniformLocation(raymarchShader.ID, "iTime"), timeValue);
        glUniform3f(glGetUniformLocation(raymarchShader.ID, "sunDirection"), dLight.direction.x, dLight.direction.y, dLight.direction.z);
        glUniform3f(glGetUniformLocation(raymarchShader.ID, "sunColor"), dLight.material.albedo.x, dLight.material.albedo.y, dLight.material.albedo.z);
        glUniform1f(glGetUniformLocation(raymarchShader.ID, "absorptionCoEff"), absorptionCoEff);
        glUniform1f(glGetUniformLocation(raymarchShader.ID, "baseMarchSize"), marchSize);
        glUniform1f(glGetUniformLocation(raymarchShader.ID, "lightMarchSize"), lightMarchSize);

        glUniformMatrix4fv(glGetUniformLocation(raymarchShader.ID, "invCameraMatrix"), 1, GL_FALSE, glm::value_ptr(glm::inverse(camera.cameraMatrix)));
        glUniform3f(glGetUniformLocation(raymarchShader.ID, "cameraPosition"), camera.Position.x, camera.Position.y, camera.Position.z);

        for (Light *light : Light::lights)
        {
            light->Draw(lightShader, pbrShader, camera, true);
        }

        for (Model *model : Model::models)
        {
            model->Draw(pbrShader, camera);
        }

        // cube.Draw(raymarchShader, camera);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Draw the framebuffer rectangle
        frameBufferShader.Activate();
        glBindVertexArray(rectVAO);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(128.0f, 0.0f, 128.0f, 255.0f), "Stats");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::DragFloat("March Size", &marchSize, 0.002f, 0.001f, 1.0f);
        ImGui::DragFloat("Light March Size", &lightMarchSize, 0.002f, 0.001f, 1.0f);
        ImGui::DragFloat("Absorption Coefficient", &absorptionCoEff, 0.01f, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        for (Light *light : Light::lights)
        {
            light->UI();
        }
        ImGui::End();

        ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        for (Model *model : Model::models)
        {
            model->UI();
        }
        cube.UI();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    for (Light *light : Light::lights)
    {
        light->SaveImGuiData("saveData/transforms.json");
    }

    for (Model *model : Model::models)
    {
        model->SaveImGuiData("saveData/transforms.json");
    }
    cube.SaveImGuiData("saveData/transforms.json");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}