#pragma once

class BeamParticles {
public:
    BeamParticles(float length, float maxRadius, int count, float bendAmount);
    void draw() const;

private:
    unsigned int VAO, VBO;
    int vertexCount;
};