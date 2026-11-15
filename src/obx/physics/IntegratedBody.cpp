#include "IntegratedBody.h"
#include "PhysicsConstants.h"

void obx::physics::IntegratedBody::applyForce(obx::physics::Force f) {
    acceleration = bx::add(acceleration, bx::mul(f.vector, 1.0f / mass));
}

// runge kutta system rk4
// https://www.youtube.com/watch?v=vNoFdtcPFdk
void obx::physics::IntegratedBody::step(float delta_time) { stepD(delta_time); }
void obx::physics::IntegratedBody::stepD(double dt) {
    const V3 r0 = r_, v0 = v_;

    const V3 k1r = v0;
    const V3 k1v = accel(r0, v0, cdAOverM, useJ2, useDrag);

    const V3 k2r = v0 + k1v * (dt * 0.5);
    const V3 k2v = accel(r0 + k1r * (dt * 0.5), k2r, cdAOverM, useJ2, useDrag);

    const V3 k3r = v0 + k2v * (dt * 0.5);
    const V3 k3v = accel(r0 + k2r * (dt * 0.5), k3r, cdAOverM, useJ2, useDrag);

    const V3 k4r = v0 + k3v * dt;
    const V3 k4v = accel(r0 + k3r * dt, k4r, cdAOverM, useJ2, useDrag);

    r_ = r0 + (k1r + k2r * 2.0 + k3r * 2.0 + k4r) * (dt / 6.0);
    v_ = v0 + (k1v + k2v * 2.0 + k3v * 2.0 + k4v) * (dt / 6.0);

    // keep the base class's float state current for everything that reads it
    position = toBx(r_);
    velocity = toBx(v_);
}

// Keplerian estimate
// note: does not in fact account for orbital decay factors

double obx::physics::IntegratedBody::getOrbitalPeriodSeconds() {

    double r = bx::length(position);
    double v = bx::length(velocity);

    double semiMajorAxis = 1.0 / ((2.0 / r) - (v * v / MU));

    // hyperbolic or parabolic trajectory
    if (semiMajorAxis <= 0.0)
        return -1.0;

    return 2.0 * M_PI * std::sqrt(std::pow(semiMajorAxis, 3) / MU);
}

// another kepler aproximation
// note: does not in fact account for orbital decay factors
bx::Vec3 obx::physics::IntegratedBody::getPositionAt(libsgp4::DateTime time) {
    constexpr double MU = 398600.4418;
    constexpr double TWO_PI = 2.0 * M_PI;

    double dt = (time - epoch).TotalSeconds();

    bx::Vec3 r0 = position;
    bx::Vec3 v0 = velocity;

    double r = bx::length(r0);
    double v2 = bx::dot(v0, v0);

    double energy = v2 * 0.5 - MU / r;

    // Semi-major axis
    double a = -MU / (2.0 * energy);

    // hyperbola = bad
    if (a <= 0.0)
        return position;

    bx::Vec3 h = bx::cross(r0, v0);

    bx::Vec3 eVec =
        bx::sub(bx::mul(bx::cross(v0, h), 1.0f / MU), bx::mul(r0, 1.0f / static_cast<float>(r)));

    double e = bx::length(eVec);

    // Orbital basis
    bx::Vec3 p{0, 0, 0};
    if (e > 1e-6) {
        p = bx::normalize(eVec);
    } else {
        p = bx::normalize(r0);
    }

    bx::Vec3 hHat = bx::normalize(h);
    bx::Vec3 q = bx::cross(hHat, p);

    // true anomaly
    double cosNu0 = bx::dot(r0, p) / r;
    double sinNu0 = bx::dot(r0, q) / r;

    double nu0 = std::atan2(sinNu0, cosNu0);

    // convert true anomaly to eccentric anomaly
    double E0 = 2.0 * std::atan2(std::sqrt(1.0 - e) * std::sin(nu0 * 0.5),
                                 std::sqrt(1.0 + e) * std::cos(nu0 * 0.5));

    // Initial mean anomaly
    double M0 = E0 - e * std::sin(E0);

    // mean motion
    double n = std::sqrt(MU / (a * a * a));

    // mean anomaly at requested time
    double M = std::fmod(M0 + n * dt, TWO_PI);

    // solve Kepler's equation: M = E - e*sin(E)
    double E = M;

    for (int i = 0; i < 10; ++i) {
        E -= (E - e * std::sin(E) - M) / (1.0 - e * std::cos(E));
    }

    // convert to usable units
    double x = a * (std::cos(E) - e);
    double y = a * std::sqrt(1.0 - e * e) * std::sin(E);

    return bx::add(bx::mul(p, static_cast<float>(x)), bx::mul(q, static_cast<float>(y)));
}
