// main.cpp
// Entry point for the pulsar simulation. Creates the window and OpenGL context,
// handles camera input, runs the main loop (physics, light curve, audio ticks,
// rendering with bloom), and draws the ImGui data/controls panel.

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm> // PRESETS
#include "Pulsar.h"
#include "Shader.h"
#include "renderer/Camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer/Sphere.h" // SPHERE
#include "renderer/BeamParticles.h" // BEAM PARTICLES
#include "renderer/Starfield.h" // STARS
#include "renderer/Bloom.h" // BLOOM
#include "audio/PulseAudio.h" // AUDIO
#include "imgui.h" // IMGUI
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// real pulsars (P in seconds, Pdot dimensionless).
// Sources: Vela P (Wikipedia), Vela Pdot (arXiv 2506.16687), B1919+21 (Pilia et al. 2016, A&A),
// J0437-4715 (Fermi 3PC; P taken as 1/F0). Crab values are NOT source-verified: check vs ATNF psrcat.
struct PulsarPreset {
    const char* name;
    double period;
    double pdot;
    const char* note;
};

static const PulsarPreset kPresets[] = {
    {"Custom (sandbox)",          0.1,        1e-15,    "Your own settings. Use the sliders below."},
    {"Crab (B0531+21)",           0.0337,     4.2e-13,  "Young and energetic, born in the supernova of 1054 CE."},
    {"Vela (B0833-45)",           0.089328,   1.24e-13, "About 11,000 years old. Famous for sudden spin-ups called glitches."},
    {"B1919+21 (first pulsar)",   1.3373,     1.349e-15,"The first pulsar ever found, discovered in 1967."},
    {"J0437-4715 (millisecond)",  0.0057575,  5.73e-20, "Nearby millisecond pulsar, spun up by a white dwarf companion."},
};
static const int kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

Camera camera;
bool mouseDown = false;
double lastX = 0.0, lastY = 0.0;

// Print GLFW errors to stderr
static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

// Esc closes the window, R resets the camera
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

// Track left-button drag for orbiting. Ignore clicks/scrolls that land on the ImGui window
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_RELEASE) { mouseDown = false; return; }
    if (!ImGui::GetIO().WantCaptureMouse) mouseDown = true;
}

// Orbit the camera while the mouse is dragged
void cursorCallback(GLFWwindow* window, double x, double y) {
    if (mouseDown) {
        camera.orbit((float)(x - lastX) * 0.3f, (float)(y - lastY) * 0.3f);
    }
    lastX = x;
    lastY = y;
}

// Zoom the camera with the scroll wheel
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    camera.zoom((float)yoffset * -0.5f);
}

// Set everything up, run the render loop until the window closes, then clean up
int main(void) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 700, "Pulsar Simulation", NULL, NULL);
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

    // Adjustable values, shown/edited in the Controls panel
    float tiltDeg = 3.0f;
    float periodSet = 0.1f;
    float bend = 0.0f;
    float spread = 0.25f;
    float flowSpeed = 0.4f; // speed of particles along the beam
    float beamWidth = 0.02f; // light-curve pulse width (radians)
    bool soundOn = true;
    bool showStars = true;
    bool bloomOn = true;          // bloom effect on/off
    float bloomThreshold = 0.2f;  // only pixels brighter than this glow
    float bloomStrength = 0.5f;   // bloom effect strength
    int bloomSize = 10;            // bloom effect: blur iterations (bigger = wider glow)
    int presetIdx = 0;        // 0 = custom sandbox
    float visualSpeed = 1.0f; // slow-motion factor (1 = real time)

    // spin axis, tilt (degrees), period (seconds), period derivative, time scale
    Pulsar pulsar(glm::vec3(0, 1, 0), tiltDeg, periodSet, 1e-15, 1e10);
    Sphere star(0.12f, 24);
    // length, maxRadius, particle count, bend amount
    BeamParticles beamParticles(5.0f, spread, 2500, bend);
    Starfield starfield(3000);

    // applyPreset: load a preset's physics into the pulsar. The spin-down speedup is chosen so the
    // period visibly creeps (0.005% per second) whatever the real spin-down rate is.
    auto applyPreset = [&](int i) {
        if (i == 0) {
            pulsar.setPeriod(periodSet);
            pulsar.setPeriodDerivative(1e-15);
            visualSpeed = 1.0f;
        } else {
            pulsar.setPeriod(kPresets[i].period);
            pulsar.setPeriodDerivative(kPresets[i].pdot);
            // Real periods are too fast to watch, so slow the visible spin to about 2 turns/second
            visualSpeed = (float)std::min(1.0, 2.0 * kPresets[i].period);
        }
        pulsar.setTimeScale(5e-5 * pulsar.getPeriod() / pulsar.getPeriodDerivative());
        pulsar.setVisualSpeed(visualSpeed);
    };

    Shader shader("src/shaders/basic.vert", "src/shaders/basic.frag");
    Shader particleShader("src/shaders/particle.vert", "src/shaders/particle.frag");
    Shader starShader("src/shaders/star.vert", "src/shaders/star.frag");

    Bloom bloom(fbWidth, fbHeight);
    int bloomW = fbWidth, bloomH = fbHeight;

    auto lastTime = std::chrono::high_resolution_clock::now();

    float prevPhase = 0.0f; // used to detect one full rotation (audio tick)

    // Light curve: ring buffer of brightness samples, sampled at 1000 Hz
    const int kSamples = 600;           // 0.6 s of history
    float lightCurve[kSamples] = {0};
    int lightOffset = 0;                // next slot to write
    float sampleAcc = 0.0f;             // leftover time since the last sample
    const float sampleDt = 0.001f;      // seconds between samples

    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        pulsar.update(dt);
        glm::vec3 beam = pulsar.getBeamDirection();

        // Physics: beam opening angle scales as 1/sqrt(period).
        // 'spread' is the beam width at a 0.2 s period; other periods scale from it.
        // Clamped so millisecond pulsars don't get absurdly wide beams on screen.
        float periodScale = std::min(4.0f, std::sqrt(0.2f / (float)pulsar.getPeriod())); // clamp
        beamParticles.setMaxRadius(spread * periodScale);

        beamParticles.update(dt, flowSpeed);

        // Light curve: fill in samples for this frame.
        // Earth sits where the beam passes at phase 0, so the dot product
        // between beam and Earth is cos^2(a) + sin^2(a) * cos(phase).
        {
            float a = pulsar.getTiltAngle();
            float c2 = std::cos(a) * std::cos(a);
            float s2 = std::sin(a) * std::sin(a);
            // the visible rotation rate includes the slow-motion factor
            float omega = (float)(2.0 * M_PI / pulsar.getPeriod()) * visualSpeed; // rad/s

            sampleAcc += dt;
            if (sampleAcc > 0.1f) sampleAcc = 0.1f; // safety cap if a frame stalls
            while (sampleAcc >= sampleDt) {
                sampleAcc -= sampleDt;
                float phi = pulsar.getPhase() - omega * sampleAcc; // phase at that sample's moment
                float d = c2 + s2 * std::cos(phi);
                float width = beamWidth * periodScale; // narrower beam at longer periods
                lightCurve[lightOffset] = std::exp(-(1.0f - d) / (width * width));
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
        if (soundOn && phase < prevPhase)
            audio.trigger(0.35f + 0.65f * pulseFactor);
        prevPhase = phase;

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        // keep the off-screen buffers matched to the window size
        if (w > 0 && h > 0 && (w != bloomW || h != bloomH)) {
            bloom.resize(w, h);
            bloomW = w;
            bloomH = h;
        }

        // draw the scene off-screen (if bloom is on), otherwise straight to the screen
        if (bloomOn) {
            bloom.beginScene();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, w, h);
        }

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 view = camera.getView();
        glm::mat4 projection = camera.getProjection((float)w / h);

        // draw first (behind everything). Strip the camera's translation from the
        // view matrix so stars behave as if infinitely far away.
        if (showStars) {
            starShader.use();
            glm::mat4 skyView = glm::mat4(glm::mat3(view));
            glUniformMatrix4fv(glGetUniformLocation(starShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
            glUniformMatrix4fv(glGetUniformLocation(starShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform1f(glGetUniformLocation(starShader.ID, "time"), (float)glfwGetTime());
            starfield.draw();
        }

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

        if (bloomOn)
            bloom.apply(bloomThreshold, bloomStrength, bloomSize);

        // --- HUD ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        double P = pulsar.getPeriod();                // seconds (real)
        double Pdot = pulsar.getPeriodDerivative();   // dimensionless (s/s, real)

        double freq = 1.0 / P;                          // Hz (real)
        double B = 3.2e19 * std::sqrt(P * Pdot);        // surface magnetic field, gauss
        double ageYears = (P / (2.0 * Pdot)) / 3.15e7;  // characteristic age, years

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::Begin("Pulsar Data");
        ImGui::Text("Pulsar:         %s", kPresets[presetIdx].name);
        ImGui::Text("Period:         %.7f s", P);
        ImGui::Text("Frequency:      %.3f Hz", freq);
        ImGui::Text("Period rate:    %.3e s/s", Pdot);
        ImGui::Text("Magnetic field: %.3e G", B);
        ImGui::Text("Char. age:      %.3e yr", ageYears);
        ImGui::Text("Pulse:          %.2f", pulseFactor);
        ImGui::Text("Playback:       %.3fx real time", visualSpeed);
        ImGui::Text("Spin-down shown %.1e x faster than real", pulsar.getTimeScale());
        ImGui::Separator();
        ImGui::Text("Light curve (Earth view)");
        ImGui::PlotLines("##lightcurve", lightCurve, kSamples, lightOffset,
                         nullptr, 0.0f, 1.0f, ImVec2(320, 80));

        if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            // pick a real pulsar or the custom sandbox
            if (ImGui::BeginCombo("Pulsar", kPresets[presetIdx].name)) {
                for (int i = 0; i < kPresetCount; i++) {
                    bool selected = (i == presetIdx);
                    if (ImGui::Selectable(kPresets[i].name, selected)) {
                        presetIdx = i;
                        applyPreset(i);
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::TextWrapped("%s", kPresets[presetIdx].note);

            if (ImGui::SliderFloat("Tilt (deg)", &tiltDeg, 0.5f, 60.0f))
                pulsar.setTiltDegrees(tiltDeg);

            // The period slider only applies to the custom sandbox; presets use real values
            ImGui::BeginDisabled(presetIdx != 0);
            if (ImGui::SliderFloat("Period (s)", &periodSet, 0.05f, 2.0f, "%.3f",
                                   ImGuiSliderFlags_Logarithmic))
                applyPreset(0);
            ImGui::EndDisabled();

            // slow motion for the visible spin (1 = real time)
            if (ImGui::SliderFloat("Playback speed", &visualSpeed, 0.001f, 1.0f, "%.3fx",
                                   ImGuiSliderFlags_Logarithmic))
                pulsar.setVisualSpeed(visualSpeed);

            if (ImGui::SliderFloat("Bend", &bend, 0.0f, 1.5f))
                beamParticles.setBend(bend);
            ImGui::SliderFloat("Beam spread (at 0.2 s)", &spread, 0.02f, 0.6f);
            ImGui::SliderFloat("Flow speed", &flowSpeed, 0.0f, 2.0f);
            ImGui::SliderFloat("Pulse width", &beamWidth, 0.005f, 0.2f);
            ImGui::Checkbox("Sound", &soundOn);
            ImGui::SameLine();
            ImGui::Checkbox("Bloom", &bloomOn);
            ImGui::SliderFloat("Bloom strength", &bloomStrength, 0.0f, 3.0f);
            ImGui::SliderFloat("Bloom threshold", &bloomThreshold, 0.05f, 1.5f);
            ImGui::SliderInt("Bloom size", &bloomSize, 1, 16);
            ImGui::Checkbox("Stars", &showStars);
            if (ImGui::Button("Reset camera")) {
                camera.yaw = 0.0f;
                camera.pitch = 30.0f;
                camera.distance = 5.0f;
            }
        }
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