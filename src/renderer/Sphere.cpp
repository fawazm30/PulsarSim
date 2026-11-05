#include "Sphere.h"
#include <glad/glad.h>
#include <cmath>

// Sphere class implementation
Sphere::Sphere(float radius, int segments) {
    // Generate vertices and indices for the sphere
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Generate vertices
    for (int y = 0; y <= segments; y++) {
        for (int x = 0; x <= segments; x++) {
            float xs = (float)x / segments;
            float ys = (float)y / segments;
            float theta = xs * 2.0f * M_PI;
            float phi = ys * M_PI;

            float px = radius * sin(phi) * cos(theta);
            float py = radius * cos(phi);
            float pz = radius * sin(phi) * sin(theta);

            vertices.push_back(px);
            vertices.push_back(py);
            vertices.push_back(pz);
        }
    }

    // Generate indices
    for (int y = 0; y < segments; y++) {
        for (int x = 0; x < segments; x++) {
            int i0 = y * (segments + 1) + x;
            int i1 = i0 + segments + 1;

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i0 + 1);

            indices.push_back(i1);
            indices.push_back(i1 + 1);
            indices.push_back(i0 + 1);
        }
    }

    // Store the number of indices for drawing
    indexCount = (int)indices.size();

    // Generate and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// Draw the sphere
void Sphere::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}