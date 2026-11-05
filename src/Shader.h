#pragma once
#include <glad/glad.h>
#include <string>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const;
    void setMat4(const std::string& name, const float* value) const;
};