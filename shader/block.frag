#version 330 core

layout(location = 0) out vec4 out_color;

in vec2 texture_coord;
in float color;
flat in vec4 ao_colors;

float biLerp(float a, float b, float c, float d, float s, float t)
{
  float x = mix(a, b, t);
  float y = mix(c, d, t);
  return mix(x, y, s);
}

void main()
{
    float interpolated_shade = biLerp(
        ao_colors.x, ao_colors.y, ao_colors.z, ao_colors.w,
        texture_coord.s, texture_coord.t
    );

    out_color = vec4(vec3(color) * interpolated_shade, 1.0f);
}
