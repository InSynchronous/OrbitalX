#pragma once

#include "ISimulated.h"
#include "PhysicsConstants.h"
#include "bx/math.h"
namespace obx::physics {

class IntegratedBody : public ISimulated {
  private:
    bx::Vec3 acceleration; // meters/second^2

    float mass; // kilograms
    libsgp4::DateTime epoch;

  public:
    IntegratedBody(float mass, bx::Vec3 position, libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position), acceleration{0}, epoch(epoch) {}

    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation, libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position, rotation), acceleration(0), epoch(epoch) {}

    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation, bx::Vec3 velocity,
                   libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position, rotation, velocity), acceleration(0), epoch(epoch) {}

    bx::Vec3 getPosition() const { return position; }
    float getMass() const { return mass; }

    void applyForce(Force force);

    void step(float delta_time) override;
    double getOrbitalPeriodSeconds() override;
    bx::Vec3 getPositionAt(libsgp4::DateTime time) override;
};

} // namespace obx::physics
