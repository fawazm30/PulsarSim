#define GLM_ENABLE_EXPERIMENTAL
#include "Pulsar.h"
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

Pulsar::Pulsar(glm::vec3 spinAxis, float tiltDegrees, float periodSeconds, float periodDerivative)
    : spinAxis(glm::normalize(spinAxis)),
      tiltAngle(glm::radians(tiltDegrees)),
      period(periodSeconds),
      periodDerivative(periodDerivative),
      currentPhase(0.0f)
{}

void Pulsar::update(float dt) {
    float angularSpeed = 2.0f * M_PI / period;  // radians per second
    currentPhase += angularSpeed * dt;
    currentPhase = fmod(currentPhase, 2.0f * M_PI);  // keep it in [0, 2π)

    period += periodDerivative * dt; // update the period based on the derivative

}

glm::vec3 Pulsar::getBeamDirection() const {
    glm::vec3 arbitrary = (fabs(spinAxis.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::vec3 referencePerp = glm::normalize(glm::cross(spinAxis, arbitrary));

    // Start from the spin axis itself, tilt it away by tiltAngle (not the other way around)
    glm::vec3 tilted = glm::rotate(spinAxis, tiltAngle, referencePerp);

    // Spin that tilted vector around spinAxis by the current phase
    glm::vec3 beam = glm::rotate(tilted, currentPhase, spinAxis);

    return glm::normalize(beam);
}