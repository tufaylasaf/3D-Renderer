#include "camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{

    glm::mat4 projection = glm::mat4(1.0f);

    view = glm::lookAt(Position, Position + Orientation, Up);

    projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

    cameraMatrix = projection * view;
}

void Camera::Matrix(Shader &shader, const char *uniform)
{

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
    glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), Position.x, Position.y, Position.z);
}

void Camera::Inputs(GLFWwindow *window)
{
    // Get ImGui I/O structure
    ImGuiIO &io = ImGui::GetIO();

    // If ImGui wants to capture the mouse, don't process camera inputs
    if (io.WantCaptureMouse)
    {
        return; // Exit the function early, no camera input when interacting with ImGui
    }

    // Camera position movement (WASD)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Position += speed * -glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Position += speed * -Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Position += speed * glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Position += speed * Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Position += speed * -Up;
    }

    // Adjust speed with shift key
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 0.004f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 0.001f;
    }

    // Check if any mouse button (left or right) is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        // Hide the cursor when any mouse button is pressed
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick)
        {
            glfwSetCursorPos(window, (width / 2), (height / 2));
            firstClick = false;
        }

        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Handle camera rotation if left mouse button is pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

            glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

            if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                Orientation = newOrientation;
            }

            Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);
        }

        // Handle camera rotation around origin if right mouse button is pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

            // Calculate the radius (distance from origin)
            float radius = glm::length(Position);

            // Horizontal rotation (yaw) around the Y axis
            float yawAngle = glm::radians(-rotY);
            Position = glm::rotateY(Position, yawAngle);

            // Vertical rotation (pitch) around the right axis (cross of up and orientation)
            glm::vec3 right = glm::normalize(glm::cross(Up, Orientation));
            float pitchAngle = glm::radians(-rotX);
            Position = glm::rotate(Position, pitchAngle, right);

            // Maintain the same distance (radius) from the origin (0,0,0)
            Position = glm::normalize(Position) * radius;

            // Always look towards the origin (0, 0, 0)
            Orientation = glm::normalize(-Position);
        }

        // Reset the mouse position to the center
        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else
    {
        // Show the cursor again when neither mouse button is pressed
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}
