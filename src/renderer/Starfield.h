#pragma once

class Starfield {
public:
    explicit Starfield(int count);
    void draw() const;

private:
    unsigned int VAO = 0, VBO = 0;
    int vertexCount;
};