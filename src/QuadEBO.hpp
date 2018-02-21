#pragma once

#include "../gl3w/gl3w.h"
#include <memory>

class QuadEBO
{
public:
    QuadEBO() {
        m_indices = 0;
        m_EBO = 0;
    }

    void bind() {
        if (m_EBO == 0) glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    }

    void resize(GLsizeiptr new_element_count) {
        // resize
        if (new_element_count <= m_indices) return;
        // get next multiple of 6
        new_element_count = ((new_element_count + 5) / 6) * 6;
        // increase by a fixed multiple of 6 amount
        new_element_count += 2 * 6;
        std::unique_ptr<GLuint[]> indices{ std::make_unique<GLuint[]>(new_element_count) };
        // fill
        GLuint a = 0;
        for (std::size_t i = 0; i < static_cast<GLuint>(new_element_count); a += 4) {
            indices[i++] = a + 0; indices[i++] = a + 1; indices[i++] = a + 2;
            indices[i++] = a + 2; indices[i++] = a + 3; indices[i++] = a + 0;
        }
        // bind
        bind();
        // upload
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * new_element_count, indices.get(), GL_STATIC_DRAW);
        // save new indice count
        m_indices = new_element_count;
    }

    GLenum type() { return GL_UNSIGNED_INT; }
    GLsizeiptr size() { return m_indices; }

private:
    GLsizeiptr m_indices;
    GLuint m_EBO;

};