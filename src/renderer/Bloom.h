// Bloom.h
// Post-processing glow: the scene is drawn off-screen, its bright parts are
// blurred, and the blur is added back on top when drawing to the screen.

#pragma once
#include "Shader.h"

class Bloom {
public:
    Bloom(int width, int height); // load bloom shaders and create the off-screen buffers
    ~Bloom();

    // Owns GPU resources, so it can't be copied
    Bloom(const Bloom&) = delete;
    Bloom& operator=(const Bloom&) = delete;

    void resize(int width, int height); // recreate buffers when the window size changes
    void beginScene(); // everything drawn after this goes into the off-screen scene texture
    void apply(float threshold, float intensity, int blurIterations); // glow + draw to the screen

private:
    void createTargets(); // allocate the scene and blur framebuffers
    void destroyTargets(); // free the scene and blur framebuffers

    int width, height;
    Shader extractShader, blurShader, compositeShader;

    unsigned int sceneFBO = 0, sceneTex = 0;
    unsigned int pingFBO[2] = {0, 0}, pingTex[2] = {0, 0};
    unsigned int quadVAO = 0;
};