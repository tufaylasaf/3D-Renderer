#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texUV;
};

class VBO
{
public:
    unsigned int ID;
    VBO(GLfloat *vertices, GLsizeiptr size);
    VBO(std::vector<Vertex> &vertices);

    void Bind();
    void Unbind();
    void Delete();
};

#endif // !VBO_CLASS_H