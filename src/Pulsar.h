// Pulsar.h
// The pulsar's physical model: spin axis, magnetic tilt, rotation period and
// spin-down rate. Tracks the current rotation phase and the beam direction.

#pragma once
#include <glm/glm.hpp>

class Pulsar {
public:
    // timeScale only speeds up spin-down (not the visible rotation)
    Pulsar(glm::vec3 spinAxis, float tiltDegrees, double periodSeconds,
           double periodDerivative, double timeScale = 1.0);

    void update(float dt); // advance rotation phase and apply spin-down
    glm::vec3 getBeamDirection() const; // unit vector of the magnetic beam right now

    double getPeriod() const { return period; } // seconds
    float getPhase() const { return (float)currentPhase; } // float: GLM rotate needs it
    glm::vec3 getSpinAxis() const { return spinAxis; }
    float getTiltAngle() const { return tiltAngle; } // radians
    double getPeriodDerivative() const { return periodDerivative; }
    double getTimeScale() const { return timeScale; }

    void setTiltDegrees(float degrees) { tiltAngle = glm::radians(degrees); }
    void setPeriod(double seconds) { period = seconds; }
    void setPeriodDerivative(double pdot) { periodDerivative = pdot; }
    void setTimeScale(double scale) { timeScale = scale; } // spin-down speedup factor
    // 1 = real time. Smaller = slow motion (only the visible rotation; the physics values are unchanged)
    void setVisualSpeed(double speed) { visualSpeed = speed; }

private:
    glm::vec3 spinAxis;
    float tiltAngle;          // radians, angle between spin axis and magnetic moment
    double period;            // seconds, current rotation period
    double periodDerivative;  // dimensionless, rate of period increase per second (Ṗ)
    double timeScale;         // spin-down speedup factor
    double currentPhase;      // radians, current rotation angle around spin axis
    double visualSpeed = 1.0; // slow-motion factor for the visible rotation
};