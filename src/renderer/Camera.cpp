#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Calculate the camera's position in Cartesian coordinates based on yaw, pitch, and distance
glm::mat4 Camera::getView() {
    float x = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    float y = distance * sin(glm::radians(pitch));
    float z = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    glm::vec3 cameraPos = glm::vec3(x, y, z);
    glm::vec3 target = glm::vec3(0.0f); // Look at the origin
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector

    return glm::lookAt(cameraPos, target, up);
}

glm::mat4 Camera::getProjection(float aspectRatio) {
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

void Camera::orbit(float dx, float dy) {
    yaw += dx;
    pitch += dy;

    // Clamp the pitch to prevent flipping
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;
}

void Camera::zoom(float delta) {
    distance += delta;
    if(distance < 1.0f) distance = 1.0f; // Prevent zooming too close
    if(distance > 100.0f) distance = 100.0f; // Prevent zooming too far
}