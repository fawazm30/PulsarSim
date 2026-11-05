#include "BeamParticles.h"
#include <glad/glad.h>
#include <vector>
#include <cstdlib>
#include <cmath>

static float randf() { return (float)rand() / RAND_MAX; }

BeamParticles::BeamParticles(float length, float maxRadius, int count) {
    std::vector<float> data; // x, y, z, t (0=near star, 1=tip)
    vertexCount = count;

    for (int i = 0; i < count; i++) {
        float t = randf();
        float radius = maxRadius * t * t; // spreads out more near the tip
        float theta = randf() * 2.0f * M_PI;
        float r = radius * sqrt(randf());

        data.push_back(r * cos(theta));
        data.push_back(t * length);
        data.push_back(r * sin(theta));
        data.push_back(t);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void BeamParticles::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
}