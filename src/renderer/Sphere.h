#pragma once
#include <vector>

// Sphere class represents a 3D sphere mesh that can be rendered using OpenGL.
class Sphere {
public:
    Sphere(float radius, int segments);
    void draw() const;

// Private members for OpenGL buffers and index count
private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
};