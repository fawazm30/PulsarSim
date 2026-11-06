// BeamParticles.cpp
// Seeds beam particles at random positions and recomputes their positions each frame.

#include "BeamParticles.h"
#include <glad/glad.h>
#include <cstdlib>
#include <cmath>

// Random float in [0, 1]
static float randf() { return (float)rand() / (float)RAND_MAX; }

// Give each particle a random angle, radius and starting point, and create the GPU buffer
BeamParticles::BeamParticles(float length_, float maxRadius_, int count, float bendAmount_)
    : length(length_), maxRadius(maxRadius_), bendAmount(bendAmount_)
{
    vertexCount = count;
    seeds.resize(count);
    buffer.resize(count * 4);

    for (int i = 0; i < count; i++) {
        seeds[i].theta = randf() * 2.0f * M_PI;
        seeds[i].radiusRand = sqrt(randf());
        seeds[i].t = randf(); // stagger starting positions so they're not bunched up
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// Move every particle outward, wrap it back to the star at the tip, and upload the new positions
void BeamParticles::update(float dt, float speed) {
    for (int i = 0; i < vertexCount; i++) {
        seeds[i].t += dt * speed;
        if (seeds[i].t > 1.0f) seeds[i].t -= 1.0f; // loop back to the star

        float t = seeds[i].t;
        float radius = maxRadius * t * t * seeds[i].radiusRand;
        float x = radius * cos(seeds[i].theta) + bendAmount * t * t;
        float z = radius * sin(seeds[i].theta);
        float y = t * length;

        buffer[i * 4 + 0] = x;
        buffer[i * 4 + 1] = y;
        buffer[i * 4 + 2] = z;
        buffer[i * 4 + 3] = t;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size() * sizeof(float), buffer.data());
}

// Draw all particles as points
void BeamParticles::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
}