#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Helper function to compile a shader from source code
static unsigned int compile(unsigned int type, const std::string& src) {
    unsigned int shader = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

// Shader class implementation
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::ifstream vFile(vertexPath), fFile(fragmentPath);
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    unsigned int vertex = compile(GL_VERTEX_SHADER, vStream.str());
    unsigned int fragment = compile(GL_FRAGMENT_SHADER, fStream.str());

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(ID, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const {
    glUseProgram(ID);
}