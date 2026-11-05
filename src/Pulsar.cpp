#define GLM_ENABLE_EXPERIMENTAL
#include "Pulsar.h"
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

Pulsar::Pulsar(glm::vec3 spinAxis, float tiltDegrees, double periodSeconds,
               double periodDerivative, double timeScale)
    : spinAxis(glm::normalize(spinAxis)),
      tiltAngle(glm::radians(tiltDegrees)),
      period(periodSeconds),
      periodDerivative(periodDerivative),
      timeScale(timeScale),
      currentPhase(0.0)
{}

void Pulsar::update(float dt) {
    double angularSpeed = 2.0 * M_PI / period;  // radians per second
    currentPhase += angularSpeed * dt;
    currentPhase = std::fmod(currentPhase, 2.0 * M_PI);  // keep it in [0, 2π)

    // Spin-down: sped up by timeScale so a realistic Ṗ is still visible
    period += periodDerivative * dt * timeScale;
}

glm::vec3 Pulsar::getBeamDirection() const {
    glm::vec3 arbitrary = (fabs(spinAxis.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::vec3 referencePerp = glm::normalize(glm::cross(spinAxis, arbitrary));

    // Start from the spin axis itself, tilt it away by tiltAngle
    glm::vec3 tilted = glm::rotate(spinAxis, tiltAngle, referencePerp);

    // Spin that tilted vector around spinAxis by the current phase
    glm::vec3 beam = glm::rotate(tilted, (float)currentPhase, spinAxis);

    return glm::normalize(beam);
}