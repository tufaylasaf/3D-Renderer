#include "light.h"

void Light::Draw(Shader &objectShader, Shader &lightShader, Camera &camera)
{
    Model::Draw(objectShader, camera); // Draw the model as usual

    // Activate the light shader
    lightShader.Activate();
    if (type == "Directional")
    {
        Light::Directional(lightShader);
    }
    else if (type == "Point")
    {
        Light::Point(lightShader);
    }
    else
    {
        Light::Spot(lightShader);
    }
}

void Light::Directional(Shader &shader)
{
    ImGui::DragFloat3("Direction", &direction[0], 0.1f);

    glUniform3f(glGetUniformLocation(shader.ID, "dLight.ambient"), material.ambient.x, material.ambient.y, material.ambient.z);
    glUniform3f(glGetUniformLocation(shader.ID, "dLight.diffuse"), material.diffuse.x, material.diffuse.y, material.diffuse.z);
    glUniform3f(glGetUniformLocation(shader.ID, "dLight.specular"), material.specular.x, material.specular.y, material.specular.z);
    glUniform3f(glGetUniformLocation(shader.ID, "dLight.direction"), direction.x, direction.y, direction.z);
}

void Light::Point(Shader &shader)
{
    ImGui::Text("Attenuation");
    ImGui::SliderFloat("pConstant", &constant, 0.0f, 1.0f);
    ImGui::SliderFloat("pLinear", &linear, 0.0f, 1.0f);
    ImGui::SliderFloat("pQuadratic", &quadratic, 0.0f, 1.0f);

    glUniform3f(glGetUniformLocation(shader.ID, "pLight[0].ambient"), material.ambient.x, material.ambient.y, material.ambient.z);
    glUniform3f(glGetUniformLocation(shader.ID, "pLight[0].diffuse"), material.diffuse.x, material.diffuse.y, material.diffuse.z);
    glUniform3f(glGetUniformLocation(shader.ID, "pLight[0].specular"), material.specular.x, material.specular.y, material.specular.z);

    glUniform3f(glGetUniformLocation(shader.ID, "pLight[0].position"), translation.x, translation.y, translation.z);
    glUniform1f(glGetUniformLocation(shader.ID, "pLight[0].constant"), constant);
    glUniform1f(glGetUniformLocation(shader.ID, "pLight[0].linear"), linear);
    glUniform1f(glGetUniformLocation(shader.ID, "pLight[0].quadratic"), quadratic);
}

void Light::Spot(Shader &shader)
{
    ImGui::Text("Attenuation");
    ImGui::SliderFloat("Constant", &constant, 0.0f, 1.0f);
    ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f);
    ImGui::Text("Cutoff Angles");
    ImGui::SliderFloat("Cutoff", &cutoff, 0.0f, 90.0f);
    ImGui::SliderFloat("Outer Cutoff", &outerCutoff, cutoff, 90.0f);

    glUniform3f(glGetUniformLocation(shader.ID, "sLight.ambient"), material.ambient.x, material.ambient.y, material.ambient.z);
    glUniform3f(glGetUniformLocation(shader.ID, "sLight.diffuse"), material.diffuse.x, material.diffuse.y, material.diffuse.z);
    glUniform3f(glGetUniformLocation(shader.ID, "sLight.specular"), material.specular.x, material.specular.y, material.specular.z);

    glUniform3f(glGetUniformLocation(shader.ID, "sLight.position"), translation.x, translation.y, translation.z);
    glUniform3f(glGetUniformLocation(shader.ID, "sLight.direction"), direction.x, direction.y, direction.z);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.constant"), constant);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.linear"), linear);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.quadratic"), quadratic);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.cutOff"), glm::cos(glm::radians(cutoff)));
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.outerCutOff"), glm::cos(glm::radians(outerCutoff)));
}
