#pragma once

#include "bx/math.h"
namespace obx::physics {

static const bx::Vec3 NULL_VECTOR = {0, 0, 0};

static const double G = 6.67430e-11;
static const double EARTH_MASS = 5.972e24;

struct Force {
    bx::Vec3 vector; // newtons
};

} // namespace obx::physics
