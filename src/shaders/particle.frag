#version 410 core
// particle.frag
// Colors beam particles, fading them out toward the beam tip.
in float vT;
out vec4 FragColor;
uniform vec3 particleColor;
// Particle color with alpha falling off along the beam
void main() {
    float alpha = mix(0.85, 0.0, vT); // fades out toward tip
    FragColor = vec4(particleColor, alpha);
}