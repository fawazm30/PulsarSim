#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <chrono>
#include "Pulsar.h"
#include "Shader.h"
#include "renderer/Camera.hpp"
#include <glm/gtc/type_ptr.hpp>

// Error callback function for GLFW
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Camera camera;
bool mouseDown = false;
double lastX = 0.0, lastY = 0.0;

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Pulsar Simulation", NULL, NULL);
    if (!window)
    {
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

    // Create a pulsar: spin axis pointing up, 30° tilt, 1 second period, slow spin-down
    Pulsar pulsar(glm::vec3(0, 1, 0), 30.0f, 1.0f, 1e-9f);

    auto lastTime = std::chrono::high_resolution_clock::now();

    Shader shader("src/shaders/basic.vert", "src/shaders/basic.frag");

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // While loop for the main rendering and update cycle
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        pulsar.update(dt);
        glm::vec3 beam = pulsar.getBeamDirection();

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float vertices[] = {
            0.0f, 0.0f, 0.0f, // Origin
            beam.x, beam.y, beam.z // Beam direction
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 6, vertices);

        shader.use();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 view = camera.getView();
        glm::mat4 projection = camera.getProjection((float)w / h);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}