// Starfield.h
// Background of randomly placed, twinkling stars on a distant sphere.

#pragma once

class Starfield {
public:
    explicit Starfield(int count); // generate `count` stars and upload them to the GPU
    void draw() const; // draw the stars as GL points

private:
    unsigned int VAO = 0, VBO = 0;
    int vertexCount;
};