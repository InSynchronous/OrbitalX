#include "IntegratedBody.h"
#include "PhysicsConstants.h"

void obx::physics::IntegratedBody::applyForce(obx::physics::Force f) {
    acceleration = bx::add(acceleration, bx::mul(f.vector, 1.0f / mass));
}

// runge kutta system rk4
// https://www.youtube.com/watch?v=vNoFdtcPFdk
void obx::physics::IntegratedBody::step(float delta_time) {

    auto gravity = [](bx::Vec3 p) -> bx::Vec3 {
        double r = bx::length(p);

        double factor = -MU / (r * r * r);

        return bx::mul(p, static_cast<float>(factor));
    };

    bx::Vec3 k1p = velocity;
    bx::Vec3 k1v = gravity(position);

    bx::Vec3 k2p = bx::add(velocity, bx::mul(k1v, delta_time * 0.5f));
    bx::Vec3 k2v = gravity(bx::add(position, bx::mul(k1p, delta_time * 0.5f)));

    bx::Vec3 k3p = bx::add(velocity, bx::mul(k2v, delta_time * 0.5f));
    bx::Vec3 k3v = gravity(bx::add(position, bx::mul(k2p, delta_time * 0.5f)));

    bx::Vec3 k4p = bx::add(velocity, bx::mul(k3v, delta_time));
    bx::Vec3 k4v = gravity(bx::add(position, bx::mul(k3p, delta_time)));

    // Update position
    bx::Vec3 positionDelta =
        bx::add(bx::add(k1p, bx::mul(k2p, 2.0f)), bx::add(bx::mul(k3p, 2.0f), k4p));

    position = bx::add(position, bx::mul(positionDelta, delta_time / 6.0f));

    bx::Vec3 velocityDelta =
        bx::add(bx::add(k1v, bx::mul(k2v, 2.0f)), bx::add(bx::mul(k3v, 2.0f), k4v));

    velocity = bx::add(velocity, bx::mul(velocityDelta, delta_time / 6.0f));
}

// Keplerian estimate
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
