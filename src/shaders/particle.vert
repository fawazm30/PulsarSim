#version 410 core
// particle.vert
// Positions beam particles and shrinks them as they travel away from the star.
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aT; // 0 = at the star, 1 = at the beam tip
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float vT;
// Transform the particle and set its point size from its distance along the beam
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vT = aT;
    gl_PointSize = mix(6.0, 1.5, aT); // bigger near the star, smaller toward the tip
}