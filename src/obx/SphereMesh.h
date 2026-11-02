#pragma once

#include "Mesh.h"
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
    SphereMesh(Vertex position, float subdivisions, float radius)
        : Mesh{position}, subdivisions(subdivisions), radius(radius) {}

    void upload() override;
};

} // namespace obx
