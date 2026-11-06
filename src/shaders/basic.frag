#version 410 core
// basic.frag
// Solid color with optional radial alpha fade. Used for the star core and its halo.
in vec3 vLocalPos;
out vec4 FragColor;
uniform vec3 objectColor;
uniform float alpha;
uniform float falloffRadius; // 0 = flat alpha (no falloff), >0 = radial glow fade
// Output the object color, fading alpha toward the edge when falloff is on
void main() {
    float a = alpha;
    if (falloffRadius > 0.0) {
        float d = length(vLocalPos.xy) / falloffRadius;
        a *= smoothstep(1.0, 0.0, d);
    }
    FragColor = vec4(objectColor, a);
}