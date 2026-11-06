// Bloom.cpp
// Implements the bloom passes: bright-pass extract, ping-pong Gaussian blur at
// half resolution, then composite of scene + glow onto the screen.

#include "Bloom.h"
#include <glad/glad.h>
#include <algorithm>
#include <iostream>

// Creates a floating-point color texture and a framebuffer that draws into it
static void makeTarget(int w, int h, unsigned int& fbo, unsigned int& tex) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Bloom: framebuffer incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Bind a texture to the given texture unit
static void bindTex(unsigned int unit, unsigned int tex) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
}

// Load the three bloom shaders and create the off-screen buffers
Bloom::Bloom(int w, int h)
    : width(w), height(h),
      extractShader("src/shaders/fullscreen.vert", "src/shaders/bloom_extract.frag"),
      blurShader("src/shaders/fullscreen.vert", "src/shaders/bloom_blur.frag"),
      compositeShader("src/shaders/fullscreen.vert", "src/shaders/bloom_composite.frag")
{
    glGenVertexArrays(1, &quadVAO); // empty VAO: the vertex shader generates the triangle itself
    createTargets();
}

// Release all framebuffers, textures and the VAO
Bloom::~Bloom() {
    destroyTargets();
    glDeleteVertexArrays(1, &quadVAO);
}

// Full-size scene target plus two half-size targets for the blur
void Bloom::createTargets() {
    makeTarget(width, height, sceneFBO, sceneTex);
    int hw = std::max(1, width / 2), hh = std::max(1, height / 2); // glow runs at half resolution
    makeTarget(hw, hh, pingFBO[0], pingTex[0]);
    makeTarget(hw, hh, pingFBO[1], pingTex[1]);
}

// Delete the scene and blur framebuffers and their textures
void Bloom::destroyTargets() {
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneTex);
    glDeleteFramebuffers(2, pingFBO);
    glDeleteTextures(2, pingTex);
}

// Rebuild the buffers at the new size (no-op if the size hasn't changed)
void Bloom::resize(int w, int h) {
    if (w == width && h == height) return;
    width = w;
    height = h;
    destroyTargets();
    createTargets();
}

// Redirect drawing into the off-screen scene texture
void Bloom::beginScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, width, height);
}

// Run extract -> blur -> composite and draw the final image to the screen
void Bloom::apply(float threshold, float intensity, int blurIterations) {
    int hw = std::max(1, width / 2), hh = std::max(1, height / 2);

    glDisable(GL_BLEND); // post-processing passes should overwrite, not add
    glBindVertexArray(quadVAO);

    // Keep only the bright parts of the scene
    glViewport(0, 0, hw, hh);
    glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[0]);
    extractShader.use();
    bindTex(0, sceneTex);
    glUniform1i(glGetUniformLocation(extractShader.ID, "scene"), 0);
    glUniform1f(glGetUniformLocation(extractShader.ID, "threshold"), threshold);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Blur back and forth between the two textures (horizontal, then vertical, repeat)
    blurShader.use();
    glUniform1i(glGetUniformLocation(blurShader.ID, "image"), 0);
    bool horizontal = true;
    for (int i = 0; i < blurIterations * 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[horizontal ? 1 : 0]);
        bindTex(0, pingTex[horizontal ? 0 : 1]);
        glUniform1i(glGetUniformLocation(blurShader.ID, "horizontal"), horizontal ? 1 : 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        horizontal = !horizontal;
    }
    // After an even number of passes, the finished glow is in pingTex[0]

    // Scene + glow, drawn to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    compositeShader.use();
    bindTex(0, sceneTex);
    bindTex(1, pingTex[0]);
    glUniform1i(glGetUniformLocation(compositeShader.ID, "scene"), 0);
    glUniform1i(glGetUniformLocation(compositeShader.ID, "bloom"), 1);
    glUniform1f(glGetUniformLocation(compositeShader.ID, "intensity"), intensity);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND); // back on for the next frame's additive scene drawing
}