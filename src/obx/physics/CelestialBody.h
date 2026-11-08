#pragma once

#include "PhysicsConstants.h"
#include "bx/math.h"
namespace obx::physics {

class CelestialBody {
  private:
    bx::Vec3 position;     // meters
    bx::Vec3 rotation;     // radians
    bx::Vec3 velocity;     // meters / second
    bx::Vec3 acceleration; // meters/second^2

    float mass; // kilograms

  public:
    CelestialBody(float mass, bx::Vec3 position)
        : mass(mass), position(position), rotation(0), velocity(0), acceleration(0) {}
    CelestialBody(float mass, bx::Vec3 position, bx::Vec3 rotation)
        : mass(mass), position(position), rotation(rotation), velocity(0), acceleration(0) {}
    CelestialBody(float mass, bx::Vec3 position, bx::Vec3 rotation, bx::Vec3 velocity)
        : mass(mass), position(position), rotation(rotation), velocity(velocity), acceleration(0) {}

    bx::Vec3 getPosition() const { return position; }
    float getMass() const { return mass; }

    void step(float delta_time);
    void applyForce(Force force);
};

} // namespace obx::physics
