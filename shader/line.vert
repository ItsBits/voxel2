#version 330 core

layout(location = 0) in vec3 Position;

uniform mat4 VP_matrix;
uniform vec3 offset;
uniform vec3 scale;

void main()
{
    gl_Position = VP_matrix * vec4(Position * scale + offset, 1.0);
}
