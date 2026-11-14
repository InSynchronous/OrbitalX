#include "SGP4Body.h"
#include "libsgp4/Eci.h"
#include "libsgp4/SGP4.h"
#include "libsgp4/Vector.h"

libsgp4::TimeSpan obx::physics::SGP4Body::ToTimeSpan(double delta_seconds) {
    return libsgp4::TimeSpan(static_cast<int64_t>(std::llround(delta_seconds * 1'000'000.0)));
}

void obx::physics::SGP4Body::step(float delta_time) {
    t = t + this->ToTimeSpan(delta_time);

    libsgp4::Eci eci = sgp4.FindPosition(t);
    libsgp4::Vector pos = eci.Position();
    libsgp4::Vector vel = eci.Velocity();

    this->position = bx::Vec3{float(pos.x), float(pos.z), float(pos.y)};
    this->velocity = bx::Vec3{float(vel.x), float(vel.z), float(vel.y)};
}

std::list<obx::physics::PassDetails>
obx::physics::SGP4Body::generatePassList(libsgp4::CoordGeodetic coordinate, int days, float dt) {

    libsgp4::DateTime d = tle.Epoch().AddDays(days);
    return obx::physics::GeneratePassList(coordinate, sgp4, tle.Epoch(), d, dt);
}

libsgp4::Eci obx::physics::SGP4Body::getPosition(libsgp4::DateTime time) {
    return sgp4.FindPosition(time);
}
