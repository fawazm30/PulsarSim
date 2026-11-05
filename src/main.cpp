#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include "Pulsar.h"
#include "Shader.h"
#include "renderer/Camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer/Sphere.h"
#include "renderer/BeamParticles.h"
#include "audio/PulseAudio.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Camera camera;
bool mouseDown = false;
double lastX = 0.0, lastY = 0.0;

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camera.yaw = 0.0f;
        camera.pitch = 30.0f;
        camera.distance = 5.0f;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        mouseDown = (action == GLFW_PRESS);
}

void cursorCallback(GLFWwindow* window, double x, double y) {
    if (mouseDown) {
        camera.orbit((float)(x - lastX) * 0.3f, (float)(y - lastY) * 0.3f);
    }
    lastX = x;
    lastY = y;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom((float)yoffset * -0.5f);
}

int main(void) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Pulsar Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glfwSwapInterval(1);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive blending

    // Dear ImGui setup (installs its own callbacks and chains ours)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    PulseAudio audio;
    audio.init();

    // spin axis, tilt (degrees), period (seconds), period derivative
    Pulsar pulsar(glm::vec3(0, 1, 0), 3.0f, 0.1, 1e-15, 1e10);
    // Sphere for the star
    Sphere star(0.12f, 24);
    // length, maxRadius, particle count, bend amount
    BeamParticles beamParticles(3.0f, 0.25f, 1500, 0.10f);

    Shader shader("src/shaders/basic.vert", "src/shaders/basic.frag");
    Shader particleShader("src/shaders/particle.vert", "src/shaders/particle.frag");

    auto lastTime = std::chrono::high_resolution_clock::now();

    float prevPhase = 0.0f; // used to detect one full rotation (audio tick)

    // LIGHT CURVE: ring buffer of brightness samples, sampled at 1000 Hz
    const int kSamples = 600;           // 0.6 s of history
    float lightCurve[kSamples] = {0};
    int lightOffset = 0;                // next slot to write
    float sampleAcc = 0.0f;             // leftover time since the last sample
    const float sampleDt = 0.001f;      // seconds between samples
    const float beamWidth = 0.02f;      // angular width of the beam in radians (tweak: bigger = wider pulse)

    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        pulsar.update(dt);
        glm::vec3 beam = pulsar.getBeamDirection();

        beamParticles.update(dt, 0.4f); // flow speed

        // LIGHT CURVE: fill in samples for this frame.
        // Earth sits where the beam passes at phase 0, so the dot product
        // between beam and Earth is cos^2(a) + sin^2(a) * cos(phase).
        {
            float a = pulsar.getTiltAngle();
            float c2 = std::cos(a) * std::cos(a);
            float s2 = std::sin(a) * std::sin(a);
            float omega = (float)(2.0 * M_PI / pulsar.getPeriod()); // rad/s

            sampleAcc += dt;
            if (sampleAcc > 0.1f) sampleAcc = 0.1f; // safety cap if a frame stalls
            while (sampleAcc >= sampleDt) {
                sampleAcc -= sampleDt;
                float phi = pulsar.getPhase() - omega * sampleAcc; // phase at that sample's moment
                float d = c2 + s2 * std::cos(phi);
                lightCurve[lightOffset] = std::exp(-(1.0f - d) / (beamWidth * beamWidth));
                lightOffset = (lightOffset + 1) % kSamples;
            }
        }

        // Pulse: spikes when either beam points toward the camera
        glm::vec3 toObserver = glm::normalize(camera.getPosition());
        glm::vec3 beamDir = glm::normalize(beam);
        float alignment = glm::max(glm::dot(beamDir, toObserver), glm::dot(-beamDir, toObserver));
        float pulseFactor = glm::pow(glm::max(alignment, 0.0f), 20.0f);

        // Tick once per rotation (phase wraps back to 0), from any camera angle.
        // Quiet in the background, louder when a beam points toward the camera.
        float phase = pulsar.getPhase();
        if (phase < prevPhase)
            audio.trigger(0.35f + 0.65f * pulseFactor);
        prevPhase = phase;

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 view = camera.getView();
        glm::mat4 projection = camera.getProjection((float)w / h);

        // --- Star: white core + pulsing blue halo ---
        shader.use();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "alpha"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "falloffRadius"), 0.0f);
        star.draw();

        float haloAlpha = 0.15f + pulseFactor * 0.6f;
        float haloScale = 2.5f + pulseFactor * 2.0f;

        glm::mat4 haloModel = glm::scale(glm::mat4(1.0f), glm::vec3(haloScale));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(haloModel));
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.3f, 0.6f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "alpha"), haloAlpha);
        glUniform1f(glGetUniformLocation(shader.ID, "falloffRadius"), 0.12f);
        star.draw();

        // --- Beam particles, both poles ---
        particleShader.use();
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(particleShader.ID, "particleColor"), 0.8f, 0.9f, 1.0f);

        // Same construction as Pulsar::getBeamDirection(): tilt first, then spin by phase
        glm::vec3 spinAxis = pulsar.getSpinAxis();
        float tiltAngle = pulsar.getTiltAngle();

        glm::vec3 arbitrary = (fabs(spinAxis.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        glm::vec3 referencePerp = glm::normalize(glm::cross(spinAxis, arbitrary));

        glm::mat4 tiltMat = glm::rotate(glm::mat4(1.0f), tiltAngle, referencePerp);
        glm::mat4 phaseMat = glm::rotate(glm::mat4(1.0f), pulsar.getPhase(), spinAxis);
        glm::mat4 model = phaseMat * tiltMat;

        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        beamParticles.draw();

        glm::mat4 modelOpposite = glm::rotate(model, (float)M_PI, glm::vec3(1, 0, 0));
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelOpposite));
        beamParticles.draw();

        // --- HUD ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        double P = pulsar.getPeriod();                // seconds
        double Pdot = pulsar.getPeriodDerivative();   // dimensionless (s/s)

        double freq = 1.0 / P;                          // Hz
        double B = 3.2e19 * std::sqrt(P * Pdot);        // surface magnetic field, gauss
        double ageYears = (P / (2.0 * Pdot)) / 3.15e7;  // characteristic age, years

        ImGui::Begin("Pulsar Data");
        ImGui::Text("Period:         %.6f s", P);
        ImGui::Text("Frequency:      %.3f Hz", freq);
        ImGui::Text("Period rate:    %.3e s/s", Pdot);
        ImGui::Text("Magnetic field: %.3e G", B);
        ImGui::Text("Char. age:      %.3e yr", ageYears);
        ImGui::Text("Pulse:          %.2f", pulseFactor);
        ImGui::Separator();
        // LIGHT CURVE: brightness seen from Earth over the last 0.6 s
        ImGui::Text("Light curve (Earth view)");
        ImGui::PlotLines("##lightcurve", lightCurve, kSamples, lightOffset,
                         nullptr, 0.0f, 1.0f, ImVec2(320, 80));
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    audio.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}