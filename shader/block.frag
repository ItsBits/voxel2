#version 330 core

layout (location = 0) out vec4 outColor;

in vec2 texture_coord;
in float color;
flat in vec4 ao_colors;

uniform sampler2D inTexture;

float sinScaled(float x) {
    x = sin(3.1416 * (x - 0.5));
    return (x + 1.0) / 2.0;
}

float biLerp(float a, float b, float c, float d, float s, float t) {
    float x = mix(a, b, t);
    float y = mix(c, d, t);
    return mix(x, y, s);
}

float smoothLerp(float a, float b, float c, float d, float s, float t) {
    float tt = smoothstep(0.0, 1.0, t);
    float ss = smoothstep(0.0, 1.0, s);
    return biLerp(a, b, c, d, ss, tt);
}

float sinLerp(float a, float b, float c, float d, float s, float t) {
    float tt = sinScaled(t);
    float ss = sinScaled(s);
    return biLerp(a, b, c, d, ss, tt);
}

void main()
{
    float interpolated_shade = smoothLerp(
        ao_colors.x, ao_colors.y, ao_colors.z, ao_colors.w,
        texture_coord.s, texture_coord.t
    );

    outColor = vec4(vec3(color * interpolated_shade), 1.0f) * texture(inTexture, texture_coord);
}
