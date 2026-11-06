// Shader.h
// Small wrapper around an OpenGL shader program built from a vertex and fragment file.

#pragma once
#include <glad/glad.h>
#include <string>

class Shader {
public:
    unsigned int ID; // OpenGL program handle
    Shader(const char* vertexPath, const char* fragmentPath); // load, compile and link both files
    void use() const; // make this the active program
    void setMat4(const std::string& name, const float* value) const; // set a mat4 uniform
};