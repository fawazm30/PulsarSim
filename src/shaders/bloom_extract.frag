#version 410 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D scene;
uniform float threshold;
void main() {
    vec3 c = texture(scene, vUV).rgb;
    float lum = max(c.r, max(c.g, c.b));
    float k = max(lum - threshold, 0.0) / max(lum, 0.0001);
    FragColor = vec4(c * k, 1.0);
}