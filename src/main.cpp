#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <chrono>
#include "Pulsar.h"
#include "Shader.h"
#include "renderer/Camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer/Sphere.h"
#include "renderer/BeamParticles.h"

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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    Pulsar pulsar(glm::vec3(0, 1, 0), 10.0f, 0.3f, 1e-9f);
    Sphere star(0.12f, 24);
    BeamParticles beamParticles(4.0f, 0.25f, 2000);

    Shader shader("src/shaders/basic.vert", "src/shaders/basic.frag");
    Shader particleShader("src/shaders/particle.vert", "src/shaders/particle.frag");

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        pulsar.update(dt);
        glm::vec3 beam = pulsar.getBeamDirection();

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 view = camera.getView();
        glm::mat4 projection = camera.getProjection((float)w / h);

        // --- Star: bright white core, then a larger translucent blue halo ---
        shader.use();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "alpha"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "falloffRadius"), 0.0f);
        star.draw();

        glm::mat4 haloModel = glm::scale(glm::mat4(1.0f), glm::vec3(2.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(haloModel));
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.3f, 0.6f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.ID, "alpha"), 0.15f);
        glUniform1f(glGetUniformLocation(shader.ID, "falloffRadius"), 0.12f);
        star.draw();

        // --- Beam particles, both poles ---
        particleShader.use();
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(particleShader.ID, "particleColor"), 0.8f, 0.9f, 1.0f);

        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 axis = glm::cross(up, beam);
        float angle = acos(glm::clamp(glm::dot(up, beam), -1.0f, 1.0f));

        glm::mat4 model = glm::mat4(1.0f);
        if (glm::length(axis) > 0.0001f)
            model = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis));

        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        beamParticles.draw();

        glm::mat4 modelOpposite = glm::rotate(model, (float)M_PI, glm::vec3(1, 0, 0));
        glUniformMatrix4fv(glGetUniformLocation(particleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelOpposite));
        beamParticles.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
