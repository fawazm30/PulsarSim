#version 410 core
// bloom_extract.frag
// First bloom pass: keeps only the parts of the scene brighter than the threshold.
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D scene;
uniform float threshold;
// Scale each pixel by how far its brightness exceeds the threshold (0 if below)
void main() {
    vec3 c = texture(scene, vUV).rgb;
    float lum = max(c.r, max(c.g, c.b));
    float k = max(lum - threshold, 0.0) / max(lum, 0.0001);
    FragColor = vec4(c * k, 1.0);
}