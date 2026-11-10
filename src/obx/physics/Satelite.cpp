#include "Satelite.h"
#include "libsgp4/Eci.h"
#include "libsgp4/SGP4.h"
#include "libsgp4/Vector.h"

libsgp4::TimeSpan obx::physics::Satelite::ToTimeSpan(float delta_ms) {
    return libsgp4::TimeSpan(static_cast<int64_t>(std::llround(delta_ms * 1000.0)));
}

void obx::physics::Satelite::step(float delta_time) {
    t = t + this->ToTimeSpan(delta_time);

    libsgp4::Eci eci = sgp4.FindPosition(t);
    libsgp4::Vector pos = eci.Position();
    libsgp4::Vector vel = eci.Velocity();

    this->position = bx::Vec3{float(pos.x), float(pos.z), float(-pos.y)};
    this->velocity = bx::Vec3{float(vel.x), float(vel.z), float(-vel.y)};
}
