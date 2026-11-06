// BeamParticles.h
// A cloud of point particles that stream outward along one pulsar beam,
// widening into a cone and optionally bending sideways.

#pragma once
#include <vector>

class BeamParticles {
public:
    BeamParticles(float length, float maxRadius, int count, float bendAmount);
    void update(float dt, float speed); // move particles along the beam and upload to the GPU
    void draw() const; // draw the particles as GL points
    void setBend(float b) { bendAmount = b; } // sideways curve at the beam tip
    void setMaxRadius(float r) { maxRadius = r; } // cone radius at the beam tip

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