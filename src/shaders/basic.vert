#version 410 core
// basic.vert
// Standard model/view/projection transform. Passes the local position on for the halo falloff.
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 vLocalPos;
// Transform the vertex to clip space
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vLocalPos = aPos;
}