#pragma once

#include "bx/math.h"
#include "libsgp4/SGP4.h"

namespace obx::physics {

class Satelite {
  private:
    bx::Vec3 position; // meters
    bx::Vec3 rotation; // radians
    bx::Vec3 velocity; // meters / second
    libsgp4::Tle tle;
    libsgp4::SGP4 sgp4;

    libsgp4::TimeSpan ToTimeSpan(float delta_ms);

  public:
    Satelite(libsgp4::Tle tle)
        : tle(tle), sgp4(tle), t(tle.Epoch()), position{0}, rotation{0}, velocity{0} {};

    libsgp4::DateTime t;

    bx::Vec3 getPosition() const { return position; }

    void step(float delta_time);
};

} // namespace obx::physics
