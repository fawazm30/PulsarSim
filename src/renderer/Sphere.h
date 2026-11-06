// Sphere.h
// UV sphere mesh, used for the neutron star and its glowing halo.

#pragma once
#include <vector>

// Sphere class represents a 3D sphere mesh that can be rendered using OpenGL.
class Sphere {
public:
    Sphere(float radius, int segments); // build the mesh and upload it to the GPU
    void draw() const; // draw the sphere as triangles

// Private members for OpenGL buffers and index count
private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
};