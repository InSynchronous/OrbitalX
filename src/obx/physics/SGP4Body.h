#pragma once

#include "ISimulated.h"
#include "Passpredict.h"
#include "bx/math.h"
#include "libsgp4/SGP4.h"
#include <list>

namespace obx::physics {

class SGP4Body : public ISimulated {
  private:
    libsgp4::Tle tle;
    libsgp4::SGP4 sgp4;

    libsgp4::TimeSpan ToTimeSpan(double delta_seconds);

  public:
    SGP4Body(libsgp4::Tle tle) : tle(tle), sgp4(tle), t(tle.Epoch()), ISimulated() {};
    SGP4Body(libsgp4::Tle tle, libsgp4::DateTime t) : tle(tle), sgp4(tle), t(t), ISimulated() {};

    libsgp4::DateTime t;

    bx::Vec3 getVelocity() const { return velocity; }
    bx::Vec3 getPosition() const { return position; }

    libsgp4::Eci getPosition(libsgp4::DateTime time);

    std::list<obx::physics::PassDetails> generatePassList(libsgp4::CoordGeodetic coordinate,
                                                          int days, float dt) override;
    const libsgp4::Tle &getTle() const { return tle; }

    void step(float delta_time) override;
};

} // namespace obx::physics
