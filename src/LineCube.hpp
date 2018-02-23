#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "../gl3w/gl3w.h"
#include "Shader.hpp"

class LineCube {
public:
    LineCube();
    ~LineCube();
    void draw(const glm::mat4 & VP, glm::vec3 offset, glm::vec3 scale);

private:
    GLuint VAO, VBO;
    GLsizei vertex_count;
    Shader shader;
    GLint offset_uniform, scale_uniform, VP_uniform;

};
