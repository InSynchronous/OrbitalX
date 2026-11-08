#pragma once

#include "Mesh.h"
#include "Vertex.h"

namespace obx::gfx_engine {
class StaticCube : public Mesh {
  private:
    float scale;
    obx::gfx_engine::Vertex vertices[8] = {
        {{-1, -1, -1}, {1, 0}}, {{1, -1, -1}, {0, 1}}, {{1, 1, -1}, {0, 0}}, {{-1, 1, -1}, {1, 1}},

        {{-1, -1, 1}, {1, 0}},  {{1, -1, 1}, {0, 1}},  {{1, 1, 1}, {1, 1}},  {{-1, 1, 1}, {1, 1}},
    };

    uint16_t indices[36] = {
        4, 5, 6, 4, 6, 7, // Front
        0, 2, 1, 0, 3, 2, // Back
        3, 7, 6, 3, 6, 2, // Top
        0, 1, 5, 0, 5, 4, // Bottom
        1, 2, 6, 1, 6, 5, // Right
        0, 4, 7, 0, 7, 3  // Left
    };

  public:
    StaticCube(bx::Vec3 position, bx::Vec3 rotation, float scale)
        : Mesh{position, rotation}, scale(scale) {}
    void upload() override;
};
} // namespace obx::gfx_engine
