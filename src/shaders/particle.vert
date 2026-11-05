#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aT;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float vT;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vT = aT;
    gl_PointSize = mix(6.0, 1.5, aT); // bigger near the star, smaller toward the tip
}