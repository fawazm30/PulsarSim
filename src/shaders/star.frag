#version 410 core
in float vBrightness;
in float vTint;
out vec4 FragColor;

void main() {
    // Soft round dot instead of a square
    float d = length(gl_PointCoord - vec2(0.5)) * 2.0;
    float a = smoothstep(1.0, 0.0, d);
    vec3 color = mix(vec3(0.7, 0.8, 1.0), vec3(1.0, 0.92, 0.8), vTint); // blue-white to warm
    FragColor = vec4(color, a * vBrightness);
}