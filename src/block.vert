#version 330 core

layout(location = 0) in uvec3 Position;
layout(location = 1) in uint Color;
layout(location = 2) in uvec4 AO;

uniform mat4 VP_matrix;
uniform vec3 offset;

out float color;
out vec2 texture_coord;
flat out vec4 ao_colors;

void main()
{
    gl_Position = VP_matrix * vec4(vec3(Position) + offset, 1.0f);
    color = float(Color) / 255.0f;

    ao_colors = AO / 255.0f;

    uvec2 i_tex_rel = uvec2((gl_VertexID & 1) ^ ((gl_VertexID >> 1) & 1), (gl_VertexID >> 1) & 1);
    texture_coord = vec2(i_tex_rel);
}
