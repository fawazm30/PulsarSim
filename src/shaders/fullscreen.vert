#version 410 core
// fullscreen.vert
// Generates one oversized triangle covering the screen, with no vertex buffer needed.
// Used by all the bloom passes.
out vec2 vUV;
// Derive the corner position and UV from gl_VertexID (0, 1, 2)
void main() {
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vUV = pos;
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}