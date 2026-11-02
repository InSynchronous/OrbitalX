#pragma once

namespace obx {
struct Vertex {
    float x, y, z;
    float r, g, b, a;

    Vertex &operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vertex &operator+=(const Vertex &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
};
} // namespace obx
