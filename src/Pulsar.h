#pragma once
#include <glm/glm.hpp>

class Pulsar {
public:
    Pulsar(glm::vec3 spinAxis, float tiltDegrees, float periodSeconds, float periodDerivative);

    void update(float dt);
    glm::vec3 getBeamDirection() const;

    float getPeriod() const { return period; }
    float getPhase() const { return currentPhase; }
    glm::vec3 getSpinAxis() const { return spinAxis; }
    float getTiltAngle() const { return tiltAngle; }

private:
    glm::vec3 spinAxis;
    float tiltAngle;        // radians, angle between spin axis and magnetic moment
    float period;           // seconds, current rotation period
    float periodDerivative; // dimensionless, rate of period increase per second (Ṗ)
    float currentPhase;     // radians, current rotation angle around spin axis
};