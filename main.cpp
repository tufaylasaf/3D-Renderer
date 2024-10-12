#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Model.h"

const unsigned int width = 1600;
const unsigned int height = 900;

const unsigned int samples = 8;

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

    gladLoadGL();

    glViewport(0, 0, width, height);

    Shader shaderProgram("res/shaders/default.vert", "res/shaders/default.frag");
    Shader lightShader("res/shaders/light.vert", "res/shaders/light.frag");

    // glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    // glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    // glm::mat4 lightModel = glm::mat4(1.0f);
    // lightModel = glm::translate(lightModel, lightPos);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    // glEnable(GL_FRAMEBUFFER_SRGB);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    Model model("res/models/monkey/monkey.gltf", "Monkey");
    Model model2("res/models/sphere/sphere.gltf", "Sphere");
    Model light("res/models/sphere/sphere.gltf", "Light");

    light.translation = glm::vec3(0.0f, 3.0f, 0.0f);
    light.scale = glm::vec3(0.1f, 0.1f, 0.1f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();
        glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), light.color.x, light.color.y, light.color.z, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), light.translation.x, light.translation.y, light.translation.z);
        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        // ImGui::TextColored(ImVec4(255.0f, 128.0f, 0.0f, 255.0f), "Directional Light");
        // ImGui::DragFloat3("Position", &light.translation[0], 0.1f);
        // ImGui::ColorEdit3("Color", &light.color[0]);

        ImGui::TextColored(ImVec4(128.0f, 0.0f, 128.0f, 255.0f), "Stats");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

        ImGui::End();

        // Draw the model
        model.Draw(shaderProgram, camera);
        model2.Draw(shaderProgram, camera);
        light.Draw(lightShader, camera);

        // Render ImGui frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    shaderProgram.Delete();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}