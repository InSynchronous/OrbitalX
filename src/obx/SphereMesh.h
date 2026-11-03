#pragma once

#include "Mesh.h"
#include "Vertex.h"
#include "bx/math.h"
#include <cstdint>
#include <vector>

namespace obx {

class SphereMesh : public Mesh {
  private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    float radius;
    int subdivisions;

  public:
    SphereMesh(bx::Vec3 position, bx::Vec3 rotation, float subdivisions,
               float radius)
        : Mesh{position, rotation}, subdivisions(subdivisions), radius(radius) {
    }

    void upload() override;
};

} // namespace obx
