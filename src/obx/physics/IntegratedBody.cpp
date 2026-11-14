#include "IntegratedBody.h"
#include "PhysicsConstants.h"

void obx::physics::IntegratedBody::applyForce(obx::physics::Force f) {
    acceleration = bx::add(acceleration, bx::mul(f.vector, 1.0f / mass));
}

void obx::physics::IntegratedBody::step(float delta_time) {
    // calculate force towards the Earth
    // note the earth itself doesn't have a simulated body; this is just for earth orbit objects
    double r = bx::length(position);

    bx::Vec3 direction = bx::normalize(bx::mul(position, -1.0f));

    double magnitude = G * EARTH_MASS * mass / (r * r);

    Force gravity(bx::mul(direction, static_cast<float>(magnitude)));

    this->applyForce(gravity);

    velocity = bx::add(velocity, bx::mul(acceleration, delta_time));
    position = bx::add(position, bx::mul(velocity, delta_time));

    acceleration = NULL_VECTOR;
}
