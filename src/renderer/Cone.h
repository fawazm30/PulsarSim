#pragma once

class Cone {
public:
    Cone(float radius, float height, int segments);
    void draw() const;

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
};