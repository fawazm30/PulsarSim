#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    float yaw = 0.0f;
    float pitch = 30.0f;   // start slightly above the disk
    float distance = 5.0f; // start at a reasonable distance

    Camera() = default;

    glm::mat4 getView(); // Calculate the camera's position in Cartesian coordinates based on yaw, pitch, and distance
    glm::mat4 getProjection(float aspectRatio); // Get the projection matrix using perspective projection
    void orbit(float dx, float dy); // Update yaw and pitch based on mouse movement to orbit around the center
    void zoom(float delta); // Update the distance from the center based on scroll input to zoom in and out
};