#pragma once

#include "../gl3w/gl3w.h"

class Texture {
public:
    Texture();
    GLuint id() { return m_texture; }

private:
    GLuint m_texture;

};
