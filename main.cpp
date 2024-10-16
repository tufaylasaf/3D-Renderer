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

    glViewport(0, 0, width, height);

    Shader shaderProgram("res/shaders/default.vert", "res/shaders/default.frag");
    Shader lightShader("res/shaders/light.vert", "res/shaders/light.frag");
    Shader skyboxShader("res/shaders/skybox.vert", "res/shaders/skybox.frag");

    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    Light dlight("res/models/Shapes/sphere.gltf", "DLight", "Directional");
    Light pLight("res/models/Shapes/sphere.gltf", "PLight", "Point");
    Light sLight("res/models/Shapes/sphere.gltf", "SLight", "Spot");

    Model sphere("res/models/Shapes/sphere.gltf", "Sphere", false);
    Model icoSphere("res/models/Shapes/icosphere.gltf", "Icosphere", false);
    Model abstract("res/models/Shapes/abstract.gltf", "Abstract", false);
    Model cube("res/models/Shapes/cube.gltf", "Cube", false);
    Model bg("res/models/Shapes/bg.gltf", "Background", false);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(128.0f, 0.0f, 128.0f, 255.0f), "Stats");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

        ImGui::End();

        // dlight.Draw(lightShader, shaderProgram, camera);
        // sLight.Draw(lightShader, shaderProgram, camera);
        // pLight.Draw(lightShader, shaderProgram, camera);

        ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        for (Light *light : Light::lights)
        {
            light->Draw(lightShader, shaderProgram, camera);
        }
        ImGui::End();

        ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        for (Model *model : Model::models)
        {
            model->Draw(shaderProgram, camera);
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    dlight.SaveImGuiData("saveData/transforms.json");
    pLight.SaveImGuiData("saveData/transforms.json");
    sLight.SaveImGuiData("saveData/transforms.json");

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