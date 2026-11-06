#version 410 core
// bloom_composite.frag
// Final bloom pass: adds the blurred glow on top of the original scene.
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D scene;
uniform sampler2D bloom;
uniform float intensity;
// Scene color + glow scaled by intensity
void main() {
    FragColor = vec4(texture(scene, vUV).rgb + texture(bloom, vUV).rgb * intensity, 1.0);
}