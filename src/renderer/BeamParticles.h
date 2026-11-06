#pragma once
#include <vector>

class BeamParticles {
public:
    BeamParticles(float length, float maxRadius, int count, float bendAmount);
    void update(float dt, float speed);
    void draw() const;
    void setBend(float b) { bendAmount = b; }
    void setMaxRadius(float r) { maxRadius = r; }

private:
    unsigned int VAO, VBO;
    int vertexCount;
    float length, maxRadius, bendAmount;

    struct ParticleSeed {
        float theta;
        float radiusRand;
        float t; // 0 = at the star, 1 = at the tip
    };
    std::vector<ParticleSeed> seeds;
    std::vector<float> buffer;
};