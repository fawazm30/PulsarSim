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
    // To find the beam direction, we need to rotate the spin axis by the tilt angle and then by the current phase.
    glm::vec3 arbitrary = (fabs(spinAxis.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::vec3 perpendicular = glm::normalize(glm::cross(spinAxis, arbitrary));

    // Now we have a perpendicular vector to the spin axis. We can rotate this vector by the tilt angle around the tilt axis (which is perpendicular to both the spin axis and the perpendicular vector).
    glm::vec3 tiltAxis = glm::normalize(glm::cross(spinAxis, perpendicular));
    glm::vec3 tilted = glm::rotate(perpendicular, tiltAngle, tiltAxis);

    // Finally, we rotate the tilted vector by the current phase around the spin axis to get the final beam direction.
    glm::vec3 beam = glm::rotate(tilted, currentPhase, spinAxis);

    return glm::normalize(beam);
}