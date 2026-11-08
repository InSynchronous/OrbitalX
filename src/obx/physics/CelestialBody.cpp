#include "CelestialBody.h"
#include "PhysicsConstants.h"

void obx::physics::CelestialBody::applyForce(obx::physics::Force f) {
    acceleration = bx::add(acceleration, bx::mul(f.vector, 1.0f / mass));
}

void obx::physics::CelestialBody::step(float delta_time) {
    velocity = bx::add(velocity, bx::mul(acceleration, delta_time));
    position = bx::add(position, bx::mul(velocity, delta_time));

    acceleration = NULL_VECTOR;
}
