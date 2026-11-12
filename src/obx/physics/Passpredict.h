#pragma once

#include "libsgp4/SGP4.h"
#include <list>

namespace obx::physics {
struct PassDetails {
    libsgp4::DateTime aos;
    libsgp4::DateTime los;
    double maxElevation;
};

std::list<obx::physics::PassDetails> GeneratePassList(const libsgp4::CoordGeodetic &userGeo,
                                                      libsgp4::SGP4 &sgp4,
                                                      const libsgp4::DateTime &startTime,
                                                      const libsgp4::DateTime &endTime,
                                                      const int timeStep);
} // namespace obx::physics
