#pragma once

#include "bx/math.h"
namespace obx::physics {

static const bx::Vec3 NULL_VECTOR = {0, 0, 0};

static const double G = 6.67430e-11;

struct Force {
    bx::Vec3 vector; // newtons
};

} // namespace obx::physics
