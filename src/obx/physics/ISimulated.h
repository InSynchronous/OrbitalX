#pragma once

/*
 * interface for all interfacable simulated objects regardless of the underlying methods
 * decendants: SGP4Simulated, NewtonSimulated
 * */

#include "Passpredict.h"
#include "bx/math.h"
#include <list>
namespace obx::physics {
class ISimulated {
  protected:
    bx::Vec3 position;
    bx::Vec3 rotation;
    bx::Vec3 velocity;

  public:
    ISimulated() : position{0}, rotation{0}, velocity{0} {}
    ISimulated(bx::Vec3 position) : position{position}, rotation{0}, velocity{0} {}
    ISimulated(bx::Vec3 position, bx::Vec3 rotation)
        : position{position}, rotation{rotation}, velocity{0} {}
    ISimulated(bx::Vec3 position, bx::Vec3 rotation, bx::Vec3 velocity)
        : position{position}, rotation{rotation}, velocity{velocity} {}

    bx::Vec3 getVelocity() const { return velocity; }
    bx::Vec3 getPosition() const { return position; }

    virtual void step(float delta_time) = 0;
    // not all things can generate a pass list, temu fix for now is to just return nothing :P
    virtual std::list<obx::physics::PassDetails> generatePassList(libsgp4::CoordGeodetic coordinate,
                                                                  int days, float dt) {
        return std::list<obx::physics::PassDetails>();
    };
};
} // namespace obx::physics
