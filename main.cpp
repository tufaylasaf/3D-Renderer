
#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "stb_image.h"
#include "camera.h"

GLfloat vertices[] =
    {
        //     COORDINATES     /        COLORS          /        NORMALS       //
        -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.0f, -1.0f, 0.0f,  // Bottom side
        -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.0f, -1.0f, 0.0f, // Bottom side
        0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.0f, -1.0f, 0.0f,  // Bottom side
        0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.0f, -1.0f, 0.0f,   // Bottom side

        -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.8f, 0.5f, 0.0f,  // Left Side
        -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, -0.8f, 0.5f, 0.0f, // Left Side
        0.0f, 0.8f, 0.0f, 0.5f, 0.0f, 0.5f, -0.8f, 0.5f, 0.0f,   // Left Side

        -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, -0.8f, // Non-facing side
        0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, -0.8f,  // Non-facing side
        0.0f, 0.8f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, -0.8f,   // Non-facing side

        0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.8f, 0.5f, 0.0f, // Right side
        0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.8f, 0.5f, 0.0f,  // Right side
        0.0f, 0.8f, 0.0f, 0.5f, 0.0f, 0.5f, 0.8f, 0.5f, 0.0f,  // Right side

        0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.8f,  // Facing side
        -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.8f, // Facing side
        0.0f, 0.8f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.8f   // Facing side
};

GLuint indices[] =
    {
        0, 1, 2,    // Bottom side
        0, 2, 3,    // Bottom side
        4, 6, 5,    // Left side
        7, 9, 8,    // Non-facing side
        10, 12, 11, // Right side
        13, 15, 14  // Facing side
};

GLfloat lightVertices[] =
    { //     COORDINATES     //
        -0.1f, -0.1f, 0.1f,
        -0.1f, -0.1f, -0.1f,
        0.1f, -0.1f, -0.1f,
        0.1f, -0.1f, 0.1f,
        -0.1f, 0.1f, 0.1f,
        -0.1f, 0.1f, -0.1f,
        0.1f, 0.1f, -0.1f,
        0.1f, 0.1f, 0.1f};

GLuint lightIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7};

const int width = 800;
const int height = 800;

int main()
{

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "3D Renderer", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    gladLoadGL();

    glViewport(0, 0, width, height);

    Shader shaderProgram("res/shaders/default.vert", "res/shaders/default.frag");

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));

    EBO EBO1(indices, sizeof(indices));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 9 * sizeof(float), (void *)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 3, GL_FLOAT, 9 * sizeof(float), (void *)(6 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    Shader lightShader("res/shaders/light.vert", "res/shaders/light.frag");

    VAO lightVAO;
    lightVAO.Bind();

    VBO lightVBO(lightVertices, sizeof(lightVertices));
    EBO lightEBO(lightIndices, sizeof(lightIndices));

    lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void *)0);

    lightVAO.Unbind();
    lightVBO.Unbind();
    lightEBO.Unbind();

    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);

    glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 pyramidModel = glm::mat4(1.0f);
    pyramidModel = glm::translate(pyramidModel, pyramidPos);

    lightShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    shaderProgram.Activate();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    glEnable(GL_DEPTH_TEST);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    while (!glfwWindowShouldClose(window))
    {

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 10.0f);

        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        VAO1.Bind();

        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0);

        lightShader.Activate();
        camera.Matrix(lightShader, "camMatrix");
        lightVAO.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();

    lightVAO.Delete();
    lightVBO.Delete();
    lightEBO.Delete();
    lightShader.Delete();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}