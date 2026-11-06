#pragma once
#include <glm/glm.hpp>

class Pulsar {
public:
    // timeScale only speeds up spin-down (not the visible rotation)
    Pulsar(glm::vec3 spinAxis, float tiltDegrees, double periodSeconds,
           double periodDerivative, double timeScale = 1.0);

    void update(float dt);
    glm::vec3 getBeamDirection() const;

    double getPeriod() const { return period; }
    float getPhase() const { return (float)currentPhase; } // float: GLM rotate needs it
    glm::vec3 getSpinAxis() const { return spinAxis; }
    float getTiltAngle() const { return tiltAngle; }
    double getPeriodDerivative() const { return periodDerivative; }
    void setTiltDegrees(float degrees) { tiltAngle = glm::radians(degrees); }
    void setPeriod(double seconds) { period = seconds; }

private:
    glm::vec3 spinAxis;
    float tiltAngle;          // radians, angle between spin axis and magnetic moment
    double period;            // seconds, current rotation period
    double periodDerivative;  // dimensionless, rate of period increase per second (Ṗ)
    double timeScale;         // spin-down speedup factor
    double currentPhase;      // radians, current rotation angle around spin axis
};