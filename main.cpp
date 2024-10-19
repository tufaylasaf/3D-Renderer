#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Model.h"
#include "light.h"

const unsigned int width = 1600;
const unsigned int height = 900;

const unsigned int samples = 8;

std::vector<Model *> Model::models;
std::vector<Light *> Light::lights;

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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, width, height);

    Shader shaderProgram("res/shaders/default.vert", "res/shaders/default.frag");
    Shader lightShader("res/shaders/light.vert", "res/shaders/light.frag");
    Shader depthShader("res/shaders/depth.vert", "res/shaders/depth.frag");

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    Light dlight("res/models/Shapes/sphere.gltf", "DLight", "Directional");
    // Light pLight("res/models/Shapes/sphere.gltf", "PLight", "Point");
    // Light sLight("res/models/Shapes/sphere.gltf", "SLight", "Spot");

    Model sphere("res/models/Shapes/sphere.gltf", "Sphere", false);
    Model icoSphere("res/models/Shapes/icosphere.gltf", "Icosphere", false);
    Model abstract("res/models/Shapes/abstract.gltf", "Abstract", false);
    Model cube("res/models/Shapes/cube.gltf", "Cube", false);
    Model bg("res/models/Shapes/bg.gltf", "Background", false);

    // ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSwapInterval(1);

    // Framebuffer for Shadow Map
    unsigned int shadowMapFBO;
    glGenFramebuffers(1, &shadowMapFBO);

    // Texture for Shadow Map FBO
    unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;
    unsigned int shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Prevents darkness outside the frustrum
    float clampColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    // Needed since we don't touch the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        // Matrices needed for the light's perspective
        glm::mat4 orthgonalProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 75.0f);
        glm::mat4 lightView = glm::lookAt(dlight.translation, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightProjection = orthgonalProjection * lightView;

        depthShader.Activate();
        glUniformMatrix4fv(glGetUniformLocation(depthShader.ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));
        // Preparations for the Shadow Map
        glViewport(0, 0, shadowMapWidth, shadowMapHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (Model *model : Model::models)
        {
            model->Draw(depthShader, camera);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, width, height);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        for (Light *light : Light::lights)
        {
            light->Draw(lightShader, shaderProgram, camera, false);
        }

        shaderProgram.Activate();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

        // Bind the Shadow Map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), 0);

        for (Model *model : Model::models)
        {
            model->Draw(shaderProgram, camera);
        }

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(128.0f, 0.0f, 128.0f, 255.0f), "Stats");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
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
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    dlight.SaveImGuiData("saveData/transforms.json");
    // pLight.SaveImGuiData("saveData/transforms.json");
    // sLight.SaveImGuiData("saveData/transforms.json");

    sphere.SaveImGuiData("saveData/transforms.json");
    icoSphere.SaveImGuiData("saveData/transforms.json");
    abstract.SaveImGuiData("saveData/transforms.json");
    cube.SaveImGuiData("saveData/transforms.json");
    bg.SaveImGuiData("saveData/transforms.json");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    shaderProgram.Delete();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
