#version 330 core

layout(location = 0) in uvec3 Position;

uniform mat4 VP_matrix;
uniform vec3 offset;

void main()
{
    gl_Position = VP_matrix * vec4(vec3(Position) + offset, 1.0f);
}
