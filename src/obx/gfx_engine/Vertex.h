#pragma once

#include "bx/math.h"

namespace obx::gfx_engine {

// Bx doesn't have a vec2, or integer variants of vectors so I added my own.
// Note this is incompatible with all bx functions, its just a neat naming
// convention
//
// I don't know how great of an idea this is, but tbh its kinda weird bx doesn't
// have it in the name of simplicity

struct UVCoord {
    float x, y;

    UVCoord operator+(const UVCoord &other) const { return {x + other.x, y + other.y}; }

    UVCoord operator-(const UVCoord &other) const { return {x - other.x, y - other.y}; }

    UVCoord operator*(float scalar) const { return {x * scalar, y * scalar}; }

    UVCoord &operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
};

struct Vertex {
    bx::Vec3 position;
    obx::gfx_engine::UVCoord uv;
};

} // namespace obx::gfx_engine
