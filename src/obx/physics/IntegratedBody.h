#pragma once

#include "ISimulated.h"
#include "PhysicsConstants.h"
#include "bx/math.h"
namespace obx::physics {

class IntegratedBody : public ISimulated {
  private:
    bx::Vec3 acceleration; // meters/second^2

    float mass; // kilograms

  public:
    IntegratedBody(float mass, bx::Vec3 position) : mass(mass), ISimulated(), acceleration{0} {}
    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation)
        : mass(mass), ISimulated(position, rotation), acceleration(0) {}
    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation, bx::Vec3 velocity)
        : mass(mass), ISimulated(position, rotation, velocity), acceleration(0) {}

    bx::Vec3 getPosition() const { return position; }
    float getMass() const { return mass; }

    void step(float delta_time);
    void applyForce(Force force);
};

} // namespace obx::physics
