#pragma once
#include "Shader.h"

class Bloom {
public:
    Bloom(int width, int height);
    ~Bloom();

    // Owns GPU resources, so it can't be copied
    Bloom(const Bloom&) = delete;
    Bloom& operator=(const Bloom&) = delete;

    void resize(int width, int height);
    void beginScene(); // everything drawn after this goes into the off-screen scene texture
    void apply(float threshold, float intensity, int blurIterations); // glow + draw to the screen

private:
    void createTargets();
    void destroyTargets();

    int width, height;
    Shader extractShader, blurShader, compositeShader;

    unsigned int sceneFBO = 0, sceneTex = 0;
    unsigned int pingFBO[2] = {0, 0}, pingTex[2] = {0, 0};
    unsigned int quadVAO = 0;
};