#version 410 core
in float vT;
out vec4 FragColor;
uniform vec3 particleColor;
void main() {
    float alpha = mix(0.85, 0.0, vT); // fades out toward tip
    FragColor = vec4(particleColor, alpha);
}