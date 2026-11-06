#include "Starfield.h"
#include <glad/glad.h>
#include <vector>
#include <cstdlib>
#include <cmath>

static float randf() { return (float)rand() / (float)RAND_MAX; }

Starfield::Starfield(int count) : vertexCount(count) {
    std::vector<float> data;
    data.reserve(count * 6);

    for (int i = 0; i < count; i++) {
        // Uniform random direction on a sphere
        float z = 2.0f * randf() - 1.0f;
        float theta = 2.0f * (float)M_PI * randf();
        float r = std::sqrt(1.0f - z * z);

        // Most stars are faint, a few are bright
        float brightness = 0.12f + 0.88f * std::pow(randf(), 6.0f);
        float size = 1.0f + 2.0f * brightness;
        float phase = randf();

        data.push_back(10.0f * r * std::cos(theta)); // x
        data.push_back(10.0f * z);                   // y
        data.push_back(10.0f * r * std::sin(theta)); // z
        data.push_back(brightness);
        data.push_back(size);
        data.push_back(phase);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    const GLsizei stride = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);
}

void Starfield::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
}