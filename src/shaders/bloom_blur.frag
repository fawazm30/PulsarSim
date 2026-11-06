#version 410 core
// bloom_blur.frag
// One direction of a separable 9-tap Gaussian blur (horizontal or vertical).
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D image;
uniform int horizontal;
const float w[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216); // Gaussian weights
// Weighted sum of the pixel and its 4 neighbors on each side
void main() {
    vec2 tex = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, vUV).rgb * w[0];
    for (int i = 1; i < 5; i++) {
        vec2 offset = (horizontal == 1) ? vec2(tex.x * i, 0.0) : vec2(0.0, tex.y * i);
        result += texture(image, vUV + offset).rgb * w[i];
        result += texture(image, vUV - offset).rgb * w[i];
    }
    FragColor = vec4(result, 1.0);
}