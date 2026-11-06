#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aBrightness;
layout (location = 2) in float aSize;
layout (location = 3) in float aPhase;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

out float vBrightness;
out float vTint;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Each star twinkles at its own rate and offset
    float twinkle = 0.85 + 0.15 * sin(time * (1.0 + fract(aPhase * 13.0) * 2.0) + aPhase * 6.2831);
    vBrightness = aBrightness * twinkle;
    vTint = fract(aPhase * 7.0);
    gl_PointSize = aSize;
}