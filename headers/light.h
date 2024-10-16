#ifndef LIGHT_CLASS_H
#define LIGHT_CLASS_H

#include "model.h"

class Light : public Model
{
public:
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float cutoff = 20.0f;
    float outerCutoff = 40.0f;

    std::string type;

    static std::vector<Light *> lights;

    Light(const char *file, std::string n, std::string t) : type(t), Model(file, n, true) { lights.push_back(this); }

    void Draw(Shader &objectShader, Shader &lightShader, Camera &camera);

private:
    void Directional(Shader &shader);

    void Point(Shader &shader);

    void Spot(Shader &shader);
};

#endif
