#pragma once

#include "ISimulated.h"
#include "PhysicsConstants.h"
#include "bx/math.h"
namespace obx::physics {

class IntegratedBody : public ISimulated {
  private:
    bx::Vec3 acceleration;

    float mass; // kilograms
    libsgp4::DateTime epoch;

    V3 r_;
    V3 v_;

  public:
    IntegratedBody(float mass, bx::Vec3 position, libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position), acceleration{0}, epoch(epoch), r_{fromBx(position)},
          v_{fromBx(velocity)} {}

    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation, libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position, rotation), acceleration(0), epoch(epoch),
          r_{fromBx(position)}, v_{fromBx(velocity)} {}

    IntegratedBody(float mass, bx::Vec3 position, bx::Vec3 rotation, bx::Vec3 velocity,
                   libsgp4::DateTime epoch)
        : mass(mass), ISimulated(position, rotation, velocity), acceleration(0), epoch(epoch),
          r_{fromBx(position)}, v_{fromBx(velocity)} {}

    bx::Vec3 getPosition() const { return position; }
    float getMass() const { return mass; }

    void applyForce(Force force);

    void step(float delta_time) override;
    double getOrbitalPeriodSeconds() override;
    bx::Vec3 getPositionAt(libsgp4::DateTime time) override;

    void stepD(double dt);
    void setState(const bx::Vec3 &pos, const bx::Vec3 &vel) {
        r_ = fromBx(pos);
        v_ = fromBx(vel);
        position = pos;
        velocity = vel;
    }

    double cdAOverM = 0.0022; // m^2/kg
    bool useJ2 = true;
    bool useDrag = true;
};

} // namespace obx::physics
