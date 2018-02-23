#include <iostream>
#include "LineCube.hpp"
#include <glm/gtc/type_ptr.hpp>

LineCube::LineCube() :
    shader{ {
        { "shader/line.vert", GL_VERTEX_SHADER },
        { "shader/line.frag", GL_FRAGMENT_SHADER }
    } }
{
    offset_uniform = glGetUniformLocation(shader.id(), "offset");
    scale_uniform = glGetUniformLocation(shader.id(), "scale");
    VP_uniform = glGetUniformLocation(shader.id(), "VP_matrix");

    constexpr float vertices[]{
        0, 0, 0, 0, 1, 0,
        1, 0, 0, 1, 1, 0,
        0, 0, 1, 0, 1, 1,
        1, 0, 1, 1, 1, 1,

        0, 1, 0, 1, 1, 0,
        1, 1, 0, 1, 1, 1,
        1, 1, 1, 0, 1, 1,
        0, 1, 1, 0, 1, 0,

        0, 0, 0, 1, 0, 0,
        1, 0, 0, 1, 0, 1,
        1, 0, 1, 0, 0, 1,
        0, 0, 1, 0, 0, 0
    };

    vertex_count = sizeof(vertices) / sizeof(vertices[0]) / 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid *)(0));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

LineCube::~LineCube() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void LineCube::draw(const glm::mat4 & VP, glm::vec3 offset, glm::vec3 scale) {
    shader.use();
    glUniformMatrix4fv(VP_uniform, 1, GL_FALSE, glm::value_ptr(VP));
    glUniform3f(offset_uniform, offset.x, offset.y, offset.z);
    glUniform3f(scale_uniform, scale.x, scale.y, scale.z);
    glLineWidth(4.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertex_count);
    glBindVertexArray(0);
}
