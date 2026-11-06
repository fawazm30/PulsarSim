// Cone.h
// Open cone mesh with its apex at the origin, pointing up the +Y axis.

#pragma once

class Cone {
public:
    Cone(float radius, float height, int segments); // build the mesh and upload it to the GPU
    void draw() const; // draw the cone as triangles

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
};