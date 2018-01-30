#pragma once

#include <vector>
#include <string>

#include "gl3w.h"

class Shader
{
public:
    // Shader source input format
    struct Source
    {
        std::string file_name;
        GLenum type;
    };

    // Constructors and assignments
    Shader() { m_id = 0; }
    Shader(const std::vector<Shader::Source> & shader_source) { load(shader_source); }
    Shader(const Shader &) = delete;
    Shader(Shader && o) : m_id{ o.m_id } { o.m_id = 0; }
    Shader & operator = (const Shader &) = delete;
    Shader & operator = (Shader && o) { glDeleteProgram(m_id); m_id = o.m_id; o.m_id = 0; return *this; }

    // Destructor
    ~Shader();

    // Reload shader
    void reload(const std::vector<Shader::Source> & shader_source);

    // Use shader
    void use() const { glUseProgram(m_id); }

    // Get shader native handle
    GLuint id() const { return m_id; }

private:
    GLuint m_id{ 0 };

    // TODO: inline error checking
    enum class Process { COMPILING, LINKING };
    static const constexpr GLsizei MAX_ERROR_LOG_LENGTH = 1024;
    bool compileLinkSuccess(GLuint shader, Shader::Process type, std::string file_name = std::string(""));

    void load(const std::vector<Shader::Source> & shader_source);

};
