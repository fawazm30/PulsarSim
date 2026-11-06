#version 410 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D scene;
uniform sampler2D bloom;
uniform float intensity;
void main() {
    FragColor = vec4(texture(scene, vUV).rgb + texture(bloom, vUV).rgb * intensity, 1.0);
}