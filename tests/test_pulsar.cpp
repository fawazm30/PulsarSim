#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include "Pulsar.h"

using Catch::Approx;

static const double kPi = 3.14159265358979323846;
static const glm::vec3 kUp(0.0f, 1.0f, 0.0f);

TEST_CASE("Phase advances by 2*pi/P * dt") {
    Pulsar p(kUp, 10.0f, 1.0, 0.0, 1.0);
    p.update(0.25f); // quarter of a rotation
    CHECK(p.getPhase() == Approx(kPi / 2).margin(1e-4));
}

TEST_CASE("Phase wraps and stays in [0, 2*pi)") {
    Pulsar p(kUp, 10.0f, 1.0, 0.0, 1.0);
    p.update(1.25f); // 1.25 rotations -> quarter turn
    CHECK(p.getPhase() == Approx(kPi / 2).margin(1e-4));

    Pulsar q(kUp, 10.0f, 0.1, 0.0, 1.0);
    for (int i = 0; i < 1000; i++) {
        q.update(0.0137f);
        REQUIRE(q.getPhase() >= 0.0f);
        REQUIRE(q.getPhase() < (float)(2.0 * kPi));
    }
}

TEST_CASE("Spin-down grows the period by Pdot * dt * timeScale") {
    Pulsar p(kUp, 10.0f, 1.0, 1e-3, 10.0);
    p.update(1.0f); // 1 + 1e-3 * 1 * 10
    CHECK(p.getPeriod() == Approx(1.01).margin(1e-9));
}

TEST_CASE("Zero tilt: beam points along the spin axis") {
    Pulsar p(kUp, 0.0f, 0.5, 0.0, 1.0);
    for (int i = 0; i < 10; i++) {
        p.update(0.1f);
        CHECK(glm::length(p.getBeamDirection() - kUp) < 1e-5f);
    }
}

TEST_CASE("Beam direction is a unit vector") {
    Pulsar p(kUp, 40.0f, 0.3, 0.0, 1.0);
    for (int i = 0; i < 50; i++) {
        p.update(0.05f);
        CHECK(glm::length(p.getBeamDirection()) == Approx(1.0f).margin(1e-5));
    }
}

TEST_CASE("Angle between beam and spin axis equals the tilt at any phase") {
    const float tilt = 30.0f;
    Pulsar p(kUp, tilt, 0.5, 0.0, 1.0);
    for (int i = 0; i < 20; i++) {
        p.update(0.07f);
        float d = glm::clamp(glm::dot(p.getBeamDirection(), kUp), -1.0f, 1.0f);
        CHECK(std::acos(d) == Approx(glm::radians(tilt)).margin(1e-4));
    }
}

TEST_CASE("Playback speed changes the visible spin, not the physics") {
    Pulsar normal(kUp, 10.0f, 1.0, 1e-3, 1.0);
    Pulsar slow(kUp, 10.0f, 1.0, 1e-3, 1.0);
    slow.setVisualSpeed(0.5);

    normal.update(0.25f);
    slow.update(0.25f);

    CHECK(slow.getPhase() == Approx(normal.getPhase() * 0.5f).margin(1e-5));
    CHECK(slow.getPeriod() == Approx(normal.getPeriod()).margin(1e-12));
}

TEST_CASE("Tiny per-frame spin-down accumulates correctly (double precision)") {
    // Millisecond pulsar with the same auto time scale main.cpp uses
    const double P = 0.0057575, Pdot = 5.73e-20;
    const double timeScale = 5e-5 * P / Pdot;
    Pulsar p(kUp, 3.0f, P, Pdot, timeScale);

    const float dt = 1.0f / 60.0f;
    const int frames = 100000;
    for (int i = 0; i < frames; i++) p.update(dt);

    // Pdot * timeScale = 5e-5 * P per second, so P grows linearly
    double expected = P * (1.0 + 5e-5 * frames * (double)dt);
    CHECK(p.getPeriod() == Approx(expected).epsilon(1e-6));
}